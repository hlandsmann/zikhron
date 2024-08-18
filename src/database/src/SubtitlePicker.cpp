#include "SubtitlePicker.h"

#include "Card.h"
#include "CbdFwd.h"
#include "IdGenerator.h"
#include "ParsingHelpers.h"
#include "Subtitle.h"
#include "WordDB.h"

#include <annotation/Tokenizer.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <memory>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
namespace ranges = std::ranges;
namespace views = std::views;

namespace database {
SubtitlePicker::SubtitlePicker(std::shared_ptr<Subtitle> _subtitle,
                               PackId _videoId,
                               std::string _videoName,
                               std::shared_ptr<CardIdGenerator> _cardIdGenerator,
                               std::shared_ptr<annotation::Tokenizer> _tokenizer,
                               std::shared_ptr<WordDB> _wordDB)
    : subtitle{std::move(_subtitle)}
    , progressFile{subtitle->getFileName().replace_extension(s_progressExtension)}
    , videoId{_videoId}
    , videoName{std::move(_videoName)}
    , cardIdGenerator{std::move(_cardIdGenerator)}
    , tokenizer{std::move(_tokenizer)}
    , wordDB{std::move(_wordDB)}
    , joinings(subtitle->getSubTexts().size(), 1)
    , cards(subtitle->getSubTexts().size(), std::weak_ptr<SubtitleCard>{})
{
    deserialize();
}

auto SubtitlePicker::getDeserializedActiveCards() -> std::vector<CardPtr>
{
    std::vector<CardPtr> deserializedActiveCards;
    ranges::transform(deserializedActiveCardIndices,
                      std::back_inserter(deserializedActiveCards),
                      [this](std::size_t index) -> CardPtr {
                          auto joinedSub = createJoinedSubtitle(index, nullptr);
                          return joinedSub.card;
                      });
    for (const auto& [index, card] : views::zip(deserializedActiveCardIndices, deserializedActiveCards)) {
        card->setActive(true);
        cards.at(index) = std::dynamic_pointer_cast<SubtitleCard>(card);
    }

    return deserializedActiveCards;
}

auto SubtitlePicker::isFrontJoinable(const CardPtr& card) const -> bool
{
    return card->getIndexInPack() > 0
           && joinings.at(card->getIndexInPack() - 1) == 1;
}

auto SubtitlePicker::isBackJoinable(const CardPtr& card) const -> bool
{
    auto joined = joinings.at(card->getIndexInPack());
    return card->getIndexInPack() + joined < joinings.size()
           && joinings.at(card->getIndexInPack() + joined) == 1;
}

auto SubtitlePicker::isSeparable(const CardPtr& card) const -> bool
{
    return joinings.at(card->getIndexInPack()) > 1;
}

auto SubtitlePicker::joinFront(const CardPtr& card) -> JoinedSubtitle
{
    if (!isFrontJoinable(card)) {
        return joinedSubtitleFromCard(card);
    }
    auto newCount = std::exchange(joinings.at(card->getIndexInPack()), 0) + 1;
    auto newIndex = card->getIndexInPack() - 1;
    joinings.at(newIndex) = newCount;
    cards.at(card->getIndexInPack()) = {};

    indices.clear();
    return createJoinedSubtitle(newIndex, nullptr);
}

auto SubtitlePicker::joinBack(const CardPtr& card) -> JoinedSubtitle
{
    if (!isBackJoinable(card)) {
        return joinedSubtitleFromCard(card);
    }
    auto index = card->getIndexInPack();
    auto& count = joinings.at(index);
    joinings.at(index + count) = 0;
    cards.at(index + count) = {};
    count++;

    indices.clear();
    return createJoinedSubtitle(index, nullptr);
}

auto SubtitlePicker::cutFront(const CardPtr& card) -> JoinedSubtitle
{
    if (!isSeparable(card)) {
        return joinedSubtitleFromCard(card);
    }
    auto newCount = std::exchange(joinings.at(card->getIndexInPack()), 1) - 1;
    auto newIndex = card->getIndexInPack() + 1;
    joinings.at(newIndex) = newCount;
    cards.at(card->getIndexInPack()) = {};

    indices.clear();
    return createJoinedSubtitle(newIndex, nullptr);
}

auto SubtitlePicker::cutBack(const CardPtr& card) -> JoinedSubtitle
{
    if (!isSeparable(card)) {
        return joinedSubtitleFromCard(card);
    }

    auto index = card->getIndexInPack();
    auto& count = joinings.at(index);
    count--;
    joinings.at(index + count) = 1;
    indices.clear();
    return createJoinedSubtitle(index, nullptr);
}

auto SubtitlePicker::getPrevious(const CardPtr& card) -> JoinedSubtitle
{
    auto index = static_cast<long>(card->getIndexInPack());
    auto rfirst = std::reverse_iterator{std::next(joinings.begin(), index)};
    auto prevIt = std::find_if(rfirst, joinings.rend(), [](const std::size_t joining) { return joining != 0; });
    if (prevIt == joinings.rend()) {
        return {};
    }
    auto nextIndex = joinings.size() - static_cast<std::size_t>(std::distance(joinings.rbegin(), prevIt)) - 1;
    auto nextCard = cards.at(nextIndex).lock();
    return createJoinedSubtitle(nextIndex, nextCard);
}

auto SubtitlePicker::getNext(const CardPtr& card) -> JoinedSubtitle
{
    auto index = static_cast<long>(card->getIndexInPack());
    auto first = std::next(joinings.begin(), index + 1);
    auto nextIt = std::find_if(first, joinings.end(), [](const std::size_t joining) { return joining != 0; });
    if (nextIt == joinings.end()) {
        return {};
    }
    auto nextIndex = static_cast<std::size_t>(std::distance(joinings.begin(), nextIt));
    auto nextCard = cards.at(nextIndex).lock();
    return createJoinedSubtitle(nextIndex, nextCard);
}

auto SubtitlePicker::hasPrevious(const CardPtr& card) -> bool
{
    auto index = card->getIndexInPack();
    return index > 0;
}

auto SubtitlePicker::hasNext(const CardPtr& card) const -> bool
{
    auto index = card->getIndexInPack();
    if (index >= joinings.size()) {
        return false;
    }
    auto first = std::next(joinings.begin(), static_cast<long>(index + 1));
    auto nextIt = std::find_if(first, joinings.end(), [](const std::size_t joining) { return joining != 0; });
    return nextIt != joinings.end();
}

auto SubtitlePicker::numberOfCards() -> std::size_t
{
    setIndices();
    return indices.size();
}

auto SubtitlePicker::getJoinedSubAt(std::size_t pos) -> JoinedSubtitle
{
    setIndices();
    auto index = indices.at(pos);
    auto card = cards.at(index).lock();
    return createJoinedSubtitle(index, card);
}

auto SubtitlePicker::createJoinedSubtitle(std::size_t index, const CardPtr& card) -> JoinedSubtitle
{
    auto count = joinings.at(index);

    const auto& subtexts = subtitle->getSubTexts();
    auto subtitleSpan = std::span{std::next(subtexts.begin(), static_cast<long>(index)), count};

    const auto& lastSub = *ranges::max_element(subtitleSpan, ranges::less{}, [](const SubText& subText) {
        return subText.startTime + subText.duration;
    });
    auto endTime = lastSub.startTime + lastSub.duration;
    auto startTime = ranges::min_element(subtitleSpan, ranges::less{}, &SubText::startTime)->startTime;

    auto newCard = std::dynamic_pointer_cast<SubtitleCard>(card);
    if (newCard == nullptr) {
        spdlog::info("ncid: {}", cardIdGenerator->getNext());
        auto cardInit = CardInit{
                .cardId = cardIdGenerator->getNext(),
                .packName = videoName,
                .packId = videoId,
                .indexInPack = index,
                .wordDB = wordDB,
                .tokenizer = tokenizer,
        };
        auto cardContent = std::vector<std::string>{};
        ranges::transform(subtitleSpan, std::back_inserter(cardContent), &SubText::text);

        newCard = std::make_shared<SubtitleCard>(std::move(cardContent), cardInit);
        cards.at(index) = newCard;
    }

    return {.card = newCard,
            .startTimeStamp = static_cast<double>(startTime) / 1000.,
            .endTimeStamp = static_cast<double>(endTime) / 1000.};
}

auto SubtitlePicker::joinedSubtitleFromCard(const CardPtr& card) -> JoinedSubtitle
{
    auto index = card->getIndexInPack();
    return createJoinedSubtitle(index, card);
}

void SubtitlePicker::save()
{
    spdlog::info("save subtitlePicker: {}", progressFile.string());
    auto out = std::ofstream{progressFile};
    out << serialize();
}

void SubtitlePicker::deserialize()
{
    if (!std::filesystem::exists(progressFile)) {
        return;
    }
    auto content = utl::load_string_file(progressFile);
    auto rest = std::string_view{content};
    verifyFileType(rest, s_progressType);

    auto version = getValue(rest, "version");
    if (version != "1.0") {
        throw std::runtime_error(fmt::format("Only version 1.0 is supported, got: {}, fn: {}", version, progressFile.string()));
    }

    joinings.clear();
    auto joiningsSv = getValue(rest, "joinings");
    while (!joiningsSv.empty()) {
        joinings.push_back(std::stoul(std::string{utl::split_front(joiningsSv, ",")}));
    }
    if (joinings.size() != subtitle->getSubTexts().size()) {
        throw std::runtime_error(fmt::format("bad sub progress format, joinings - got: {}, expected: {}, fn: {}",
                                             joinings.size(), subtitle->getSubTexts().size(), progressFile.string()));
    }
    auto activeSv = getValue(rest, "active");
    while (!activeSv.empty()) {
        const auto& active = std::string{utl::split_front(activeSv, ",")};
        if (!active.empty()) {
            deserializedActiveCardIndices.push_back(std::stoul(active));
        }
    }
}

auto SubtitlePicker::serialize() const -> std::string
{
    std::string content;
    content += fmt::format("{};version:1.0\n", s_progressType);
    content += fmt::format("joinings:{},\n", fmt::join(joinings, ","));

    std::vector<std::size_t> active;
    for (const auto& [joining, card] : views::zip(joinings, cards)) {
        if (joining == 0 || !card.lock() || !card.lock()->isActive()) {
            continue;
        }
        active.push_back(card.lock()->getIndexInPack());
    }
    content += fmt::format("active:{},\n", fmt::join(active, ","));

    return content;
}

void SubtitlePicker::setIndices()
{
    if (!indices.empty()) {
        return;
    }

    for (const auto [i, joining] : views::enumerate(joinings)) {
        if (joining != 0) {
            indices.push_back(static_cast<std::size_t>(i));
        }
    }
}
} // namespace database
