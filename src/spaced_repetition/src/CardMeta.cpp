#include "CardMeta.h"

#include <CardContent.h>
#include <annotation/Ease.h>
#include <annotation/Token.h>
#include <annotation/TokenText.h>
#include <database/Card.h>
#include <database/CbdFwd.h>
#include <dictionary/DictionaryChi.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <srtypes.h>
#include <utils/index_map.h>
#include <utils/min_element_val.h>

#include <algorithm>
#include <cstddef>
#include <exception>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

#include <sys/types.h>

namespace ranges = std::ranges;
namespace views = std::views;

namespace sr {

CardMeta::CardMeta(CardId _cardId,
                   std::shared_ptr<database::Card> _card,
                   std::shared_ptr<utl::index_map<VocableId, VocableMeta>> _vocables)
    : cardId{_cardId}
    , card{std::move(_card)}
    , vocables{std::move(_vocables)}
{}

auto CardMeta::isValid() const -> bool
{
    return card != nullptr;
}

auto CardMeta::getCardId() const -> CardId
{
    return cardId;
}

auto CardMeta::getCard() const -> database::CardPtr
{
    return card;
}

auto CardMeta::VocableIndices() const -> const index_set&
{
    if (optVocableIndices.has_value()) {
        return *optVocableIndices;
    }
    return optVocableIndices.emplace(generateVocableIndexes());
}

auto CardMeta::VocableIds() const -> const vocId_set&
{
    if (optVocableIds.has_value()) {
        return *optVocableIds;
    }
    auto vocableIds = generateVocableIDs();
    return optVocableIds.emplace(vocableIds.begin(), vocableIds.end());
}

auto CardMeta::NewVocableIds() const -> vocId_set
{
    const auto& vocableIds = VocableIds();
    vocId_set newVocableIds;
    ranges::copy_if(vocableIds,
                    std::inserter(newVocableIds, newVocableIds.begin()),
                    [this](VocableId vocId) { return not vocables->contains(vocId); });
    return newVocableIds;
}

auto CardMeta::getTimingAndVocables(CardContent cardContent) const -> const TimingAndVocables&
{
    auto& tav = [this, cardContent]() -> std::optional<TimingAndVocables>& {
        switch (cardContent) {
        case CardContent::normal:
            return timingAndVocables;
        case sr::CardContent::pulled:
            return timingAndVocablesPulled;
        case sr::CardContent::inactiveVisible:
            return timingAndVocablesInactiveVisible;
        }
        std::unreachable();
    }();
    if (tav.has_value()) {
        return *tav;
    }
    return tav.emplace(generateTimingAndVocables(cardContent));
}

void CardMeta::resetTimingAndVocables()
{
    timingAndVocablesPulled.reset();
    timingAndVocables.reset();
}

auto CardMeta::getStudyTokenText() -> std::unique_ptr<annotation::TokenText>
{
    return std::make_unique<annotation::TokenText>(card);
}

// auto CardMeta::getRelevantEase() const -> std::map<VocableId, Ease>
// {
//     std::vector<VocableId> vocableIds = getActiveVocableIds();
//     std::vector<Ease> eases = easesFromVocableIds(vocableIds);
//     std::map<VocableId, Ease> relevantEases;
//
//     ranges::transform(vocableIds, eases,
//                       std::inserter(relevantEases, relevantEases.begin()),
//                       [](VocableId vocId, Ease ease) -> std::pair<VocableId, Ease> {
//                           return {vocId, ease};
//                       });
//     return relevantEases;
// }

void CardMeta::resetMetaData()
{
    optVocableIndices.reset();
    optVocableIds.reset();
    timingAndVocables.reset();
    timingAndVocablesPulled.reset();
}

auto CardMeta::generateTimingAndVocables(CardContent cardContent) const -> TimingAndVocables
{
    using SpacedRepetitionData = database::SpacedRepetitionData;
    auto filter_enabled = [this](std::size_t vocableIndex) { return (*vocables)[vocableIndex].SpacedRepetitionData()->enabled; };
    const auto& srd = [this](std::size_t vocableIndex) { return *(*vocables)[vocableIndex].SpacedRepetitionData(); };
    const auto& vocableIndices = cardContent == CardContent::inactiveVisible ? VocableIndices()
                                                                             : std::ranges::to<index_set>(VocableIndices()
                                                                                                          | views::filter(filter_enabled));
    if (vocableIndices.empty()) {
        return {};
    }

    SpacedRepetitionData threshHoldProgress{};
    if (cardContent == CardContent::normal) {
        auto progresses = vocableIndices | views::transform(srd);
        auto minIt = ranges::min_element(progresses, std::less{}, &SpacedRepetitionData::getRepeatRange);
        threshHoldProgress = *minIt;
    }
    index_set nextActiveVocables;
    ranges::copy_if(vocableIndices,
                    std::inserter(nextActiveVocables, nextActiveVocables.begin()),
                    [&](const auto& vocableIndex) {
                        auto getRepeatRange = (*vocables)[vocableIndex].SpacedRepetitionData()->getRepeatRange();
                        return threshHoldProgress.getRepeatRange().implies(getRepeatRange);
                    });
    if (nextActiveVocables.empty()) {
        return {};
    }

    auto progressesNextActive = nextActiveVocables | views::transform(srd);
    auto nextActiveIt = ranges::max_element(
            progressesNextActive,
            [](const auto& range1, const auto& range2) -> bool {
                return range1.daysMin < range2.daysMin;
            },
            &SpacedRepetitionData::getRepeatRange);
    const auto& nextActiveProgress = *nextActiveIt;
    if (cardContent == CardContent::normal) {
        auto minNormalIt = ranges::min_element(
                progressesNextActive,
                [](const auto& range1, const auto& range2) -> bool {
                    return range1.daysNormal < range2.daysNormal;
                },
                &SpacedRepetitionData::getRepeatRange);
        const auto& minNormalProgress = *minNormalIt;
        auto minMaxIt = ranges::min_element(
                progressesNextActive,
                [](const auto& range1, const auto& range2) -> bool {
                    return range1.daysMax < range2.daysMax;
                },
                &SpacedRepetitionData::getRepeatRange);
        const auto& minMaxProgress = *minMaxIt;
        auto timing = minNormalProgress.getRepeatRange().daysNormal;
        if (minMaxProgress.getRepeatRange().daysMax > 0
            && timing > nextActiveProgress.getRepeatRange().daysMin) {
            return {.timing = timing, .vocables = {}};
        }
    }

    auto timing = nextActiveProgress.getRepeatRange().daysMin;
    if (timing > 0) {
        return {.timing = timing, .vocables = {}};
    }
    return {.timing = timing,
            .vocables = nextActiveVocables};
}

auto CardMeta::generateVocableIDs() const -> std::vector<VocableId>
{
    const auto& tokens = card->getTokens();
    auto vocableIds = std::vector<VocableId>{};
    ranges::transform(tokens | std::views::filter([](const annotation::Token& token) {
                          return token.getWord() != nullptr;
                      }),
                      std::inserter(vocableIds, vocableIds.begin()),
                      [](const annotation::Token& token) -> VocableId {
                          auto vocId = token.getWord()->getId();
                          return vocId;
                      });
    return vocableIds;
}

auto CardMeta::generateVocableIndexes() const -> index_set
{
    index_set result;
    const auto& vocableIds = VocableIds();
    try {
        ranges::transform(vocableIds,
                          std::inserter(result, result.begin()),
                          [this](VocableId vocableId) -> std::size_t {
                              return vocables->index_at_id(vocableId);
                          });
    } catch (const std::exception& e) {
        spdlog::error("Failed to get Vocable Indexes, error: {}", e.what());
        return {};
    }
    return result;
}

auto CardMeta::getActiveVocableIds() const -> std::vector<VocableId>
{
    const auto& activeVocableIndices = getTimingAndVocables(CardContent::pulled).vocables;
    std::vector<VocableId> activeVocableIds;
    ranges::transform(activeVocableIndices, std::inserter(activeVocableIds, activeVocableIds.begin()),
                      [this](size_t vocableIndex) -> VocableId {
                          return vocables->id_from_index(vocableIndex);
                      });

    return activeVocableIds;
}

} // namespace sr
