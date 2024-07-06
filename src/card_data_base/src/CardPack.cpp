#include "CardPack.h"

#include "Card.h"
#include "CbdFwd.h"

#include <annotation/Tokenizer.h>
#include <dictionary/WordDB.h>
#include <misc/Identifier.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <cstddef>
#include <exception>
#include <filesystem>
#include <magic_enum.hpp>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace annotation {

CardPack::CardPack(std::filesystem::path _filename,
                   PackId _packId,
                   std::shared_ptr<WordDB> _wordDB,
                   std::shared_ptr<Tokenizer> _tokenizer)
    : filename{std::move(_filename)}
    , packId{_packId}
    , wordDB{std::move(_wordDB)}
    , tokenizer{std::move(_tokenizer)}
{
    try {
        deserialize();
    } catch (const std::exception& e) {
        spdlog::error(e.what());
        spdlog::error("Failed to load '{}'", filename.string());
    }
}

auto CardPack::getCards() const -> const std::vector<CardAudio>&
{
    return cards;
}

auto CardPack::getCardByIndex(std::size_t index) const -> const CardAudio&
{
    return cards.at(index);
}

auto CardPack::getFirstCard() const -> const CardAudio&
{
    return cards.front();
}

auto CardPack::getLastCard() const -> const CardAudio&
{
    return cards.back();
}

auto CardPack::getNextCard(const CardPtr& card) const -> std::optional<CardAudio>
{
    if (card->getPackId() != packId) {
        return {};
    }
    auto indexInPack = card->getIndexInPack();
    if (indexInPack + 1 < cards.size()) {
        return {cards.at(indexInPack + 1)};
    }
    return {};
}

auto CardPack::getPreviousCard(const CardPtr& card) const -> std::optional<CardAudio>
{
    if (card->getPackId() != packId) {
        return {};
    }
    auto indexInPack = card->getIndexInPack();
    if (indexInPack > 0) {
        return {cards.at(indexInPack - 1)};
    }
    return {};
}

auto CardPack::getName() const -> std::string
{
    return name;
}

void CardPack::deserialize()
{
    auto content = utl::load_string_file(filename);
    auto rest = std::string_view{content};
    auto cardPackType = utl::split_front(rest, ';');

    auto optType = magic_enum::enum_cast<CardPackType>(cardPackType);
    if (!optType.has_value()) {
        throw std::runtime_error(fmt::format("Invalid CardPack. Failed to parse type: {}", cardPackType));
    }
    type = optType.value();

    auto versionString = utl::split_front(rest, ':');
    if (versionString != "version") {
        throw std::runtime_error(fmt::format("Expected \"version\", got: {}", versionString));
    }
    auto version = utl::split_front(rest, '\n');
    if (version != "1.0") {
        throw std::runtime_error(fmt::format("Only version 1.0 is supported, got: {}", version));
    }
    auto nameString = utl::split_front(rest, ':');
    if (nameString != "name") {
        throw std::runtime_error(fmt::format("'name' expected, got: '{}'", nameString));
    }
    name = utl::split_front(rest, '\n');

    std::string_view audioFile;

    auto cardInit = CardInit{.packName = name,
                             .packId = packId,
                             .indexInPack = 0,
                             .wordDB = wordDB,
                             .tokenizer = tokenizer};
    while (!rest.empty()) {
        auto unknown = utl::split_front(rest, ':');
        if (unknown == "audio") {
            audioFile = utl::split_front(rest, '\n');
            unknown = utl::split_front(rest, ':');
        }
        std::string start{unknown};
        std::string end{utl::split_front(rest, ';')};
        auto cardText = utl::split_front(rest, '\n');

        cards.push_back({.audioFile = audioFile,
                         .card = Card::deserializeCard(cardText, cardInit),
                         .start = std::stod(start),
                         .end = std::stod(end)});
        cardInit.indexInPack++;
    }
}
} // namespace annotation
