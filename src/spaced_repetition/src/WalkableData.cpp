#include <CardProgress.h>
#include <VocableProgress.h>
#include <WalkableData.h>
#include <annotation/Card.h>
#include <annotation/Ease.h>
#include <annotation/ZH_Annotator.h>
#include <bits/ranges_algo.h>
#include <dictionary/ZH_Dictionary.h>
#include <folly/sorted_vector_types.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <utils/index_map.h>
#include <utils/min_element_val.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <ranges>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <sys/types.h>

namespace ranges = std::ranges;
namespace views = std::views;
namespace sr {
VocableMeta::VocableMeta(VocableProgress _progress,
                         folly::sorted_vector_set<std::size_t> _cardIndices,
                         ZH_Annotator::ZH_dicItemVec _dicItemVec)
    : progress{std::move(_progress)}
    , cardIndices{std::move(_cardIndices)}
    , dicItemVec{std::move(_dicItemVec)} {}

auto VocableMeta::Progress() const -> const VocableProgress&
{
    return progress;
}

auto VocableMeta::CardIndices() const -> const folly::sorted_vector_set<std::size_t>&
{
    return cardIndices;
}

void VocableMeta::advanceByEase(Ease ease)
{
    progress.advanceByEase(ease);
}

void VocableMeta::cardIndices_insert(std::size_t cardIndex)
{
    cardIndices.insert(cardIndex);
}

CardMeta::CardMeta(folly::sorted_vector_set<std::size_t> _vocableIndices,
                   std::reference_wrapper<utl::index_map<VocableId, VocableMeta>> _refVocables)
    : vocableIndices{std::move(_vocableIndices)}
    , refVocables{_refVocables}
{}

auto CardMeta::VocableIndices() const -> const folly::sorted_vector_set<std::size_t>&
{
    return vocableIndices;
}

void CardMeta::vocableIndices_insert(std::size_t vocableIndex)
{
    vocableIndices.insert(vocableIndex);
}

auto CardMeta::getTimingAndVocables(bool pull) const -> const TimingAndVocables&
{
    auto& tav = pull ? timingAndVocablesPulled : timingAndVocables;
    if (tav.has_value()) {
        return *tav;
    }
    return tav.emplace(generateTimingAndVocables(pull));
}

void CardMeta::resetTimingAndVocables()
{
    timingAndVocablesPulled.reset();
    timingAndVocables.reset();
}

auto CardMeta::generateTimingAndVocables(bool pull) const -> TimingAndVocables
{
    const auto& vocables = refVocables.get();
    auto vocable_progress = [&vocables](std::size_t vocableIndex) { return vocables[vocableIndex].Progress(); };

    VocableProgress threshHoldProgress{};
    if (not pull) {
        auto progresses = vocableIndices | views::transform(vocable_progress);
        auto minIt = ranges::min_element(progresses, std::less{}, &VocableProgress::getRepeatRange);
        threshHoldProgress = *minIt;
    }
    folly::sorted_vector_set<std::size_t> nextActiveVocables;
    ranges::copy_if(vocableIndices, std::inserter(nextActiveVocables, nextActiveVocables.begin()),
                    [&](const auto& vocableIndex) {
                        auto getRepeatRange = vocables[vocableIndex].Progress().getRepeatRange();
                        return threshHoldProgress.getRepeatRange().implies(getRepeatRange);
                    });
    if (nextActiveVocables.empty()) {
        return {};
    }

    auto progressesNextActive = nextActiveVocables | views::transform(vocable_progress);
    auto nextActiveIt = ranges::max_element(
            progressesNextActive,
            [](const auto& range1, const auto& range2) -> bool {
                return range1.daysMin < range2.daysMin;
            },
            &VocableProgress::getRepeatRange);
    const auto& nextActiveProgress = *nextActiveIt;
    auto timing = nextActiveProgress.getRepeatRange().daysMin;
    if (timing > 0) {
        return {};
    }
    return {.timing = timing,
            .vocables = nextActiveVocables};
}

WalkableData::WalkableData(std::shared_ptr<zikhron::Config> config)
    : db{std::move(config)}
{
    fillIndexMaps();
}

auto WalkableData::Vocables() const -> const utl::index_map<VocableId, VocableMeta>&
{
    return vocables;
}

auto WalkableData::Cards() -> utl::index_map<CardId, CardMeta>&
{
    return cards;
}

auto WalkableData::getCardCopy(size_t cardIndex) const -> CardDB::CardPtr
{
    CardId cardId = cards.id_from_index(cardIndex);
    const CardDB::CardPtr& cardPtr = db.getCards().at(cardId);
    return cardPtr->clone();
}

auto WalkableData::getVocableIdsInOrder(size_t cardIndex) const -> std::vector<VocableId>
{
    CardId cardId = cards.id_from_index(cardIndex);
    const CardDB::CardPtr& cardPtr = db.getCards().at(cardId);
    const auto& vocableChoices = db.VocableChoices();
    const ZH_Annotator& annotator = cardPtr->getAnnotator();
    std::vector<VocableId> vocableIds;
    ranges::transform(annotator.Items() | std::views::filter([](const ZH_Annotator::Item& item) {
                          return not item.dicItemVec.empty();
                      }),
                      std::back_inserter(vocableIds),
                      [&](const ZH_Annotator::Item& item) -> VocableId {
                          // TODO remove static_cast
                          auto vocId = static_cast<VocableId>(item.dicItemVec.front().id);
                          if (const auto it = vocableChoices.find(vocId);
                              it != vocableChoices.end()) {
                              // TODO remove static_cast
                              vocId = static_cast<VocableId>(it->second);
                          }
                          return vocId;
                      });
    return vocableIds;
}

auto WalkableData::getActiveVocables(size_t cardIndex) -> std::set<VocableId>
{
    const auto& activeVocableIndices = cards[cardIndex].getTimingAndVocables(true).vocables;
    std::set<VocableId> activeVocableIds;
    ranges::transform(activeVocableIndices, std::inserter(activeVocableIds, activeVocableIds.begin()),
                      [this](size_t vocableIndex) -> VocableId {
                          return vocables.id_from_index(vocableIndex);
                      });

    return activeVocableIds;
}

auto WalkableData::getRelevantEase(size_t cardIndex) -> std::map<VocableId, Ease>
{
    std::set<VocableId> activeVocables = getActiveVocables(cardIndex);
    std::map<VocableId, Ease> ease;
    ranges::transform(
            activeVocables,
            std::inserter(ease, ease.begin()),
            [&, this](VocableId vocId) -> std::pair<VocableId, Ease> {
                const VocableProgress& vocSR = vocables.at_id(vocId).second.Progress();
                // const VocableProgress vocSR = id_vocableSR.contains(vocId) ? id_vocableSR.at(vocId) : VocableProgress{};
                spdlog::debug("Easefactor of {} is {:.2f}, invervalDay {:.2f} - id: {}",
                              db.Dictionary()->EntryFromPosition(vocId, CharacterSetType::Simplified).key,
                              vocSR.EaseFactor(),
                              vocSR.IntervalDay(),
                              vocId);
                return {vocId, {vocSR.IntervalDay(), vocSR.EaseFactor(), vocSR.IndirectIntervalDay()}};
            });
    return ease;
}

void WalkableData::setEaseVocable(VocableId vocId, Ease ease)
{
    VocableMeta& vocable = vocables.at_id(vocId).second;
    vocable.advanceByEase(ease);
}

void WalkableData::resetCardsContainingVocable(VocableId vocId)
{
    const auto& [_, vocable] = vocables.at_id(vocId);
    const auto& cardIndices = vocable.CardIndices();
    for (size_t card_index : cardIndices) {
        auto& card = cards[card_index];
        card.resetTimingAndVocables();
    }
}

void WalkableData::fillIndexMaps()
{
    for (const auto& [_, card] : db.getCards()) {
        insertVocabularyOfCard(card);
    }
    spdlog::info("number of vocables: {}", vocables.size());
    spdlog::info("number of cards: {}", cards.size());
}

void WalkableData::insertVocabularyOfCard(const CardDB::CardPtr& card)
{
    const ZH_Annotator& annotator = card->getAnnotator();
    std::map<std::string, uint> zhdic_vocableMeta;
    // Its unfortunate, that we cannot simply use a view.... but we gotta live with that.
    // So lets create a temporary vector annotatorItems to represent that view.
    std::vector<std::reference_wrapper<const ZH_Annotator::ZH_dicItemVec>> annotatorItems;
    ranges::transform(annotator.Items() | views::filter([](const ZH_Annotator::Item& item) {
                          return not item.dicItemVec.empty();
                      }),
                      std::back_inserter(annotatorItems),
                      [](const auto& item) -> std::reference_wrapper<const ZH_Annotator::ZH_dicItemVec> {
                          return item.dicItemVec;
                      });

    // TODO remove static cast
    auto [card_index, cardMetaRef] = cards.emplace(static_cast<CardId>(card->Id()),
                                                   CardMeta{folly::sorted_vector_set<std::size_t>{},
                                                            std::ref(vocables)});
    auto& cardMeta = cardMetaRef.get();
    std::vector<VocableId> vocableIds = getVocableIdsInOrder(card, db.VocableChoices());
    for (const auto& [vocId, dicItemVec] : views::zip(vocableIds, annotatorItems)) {
        const auto& optionalIndex = vocables.optional_index(vocId);
        if (optionalIndex.has_value()) {
            auto& vocable = vocables[*optionalIndex];
            vocable.cardIndices_insert(card_index);
            cardMeta.vocableIndices_insert(*optionalIndex);
        } else {
            const auto& progressVocables = db.ProgressVocables();
            auto itVoc = progressVocables.find(vocId);
            const auto& [vocable_index, _] = vocables.emplace(vocId,
                                                              (itVoc != progressVocables.end())
                                                                      ? itVoc->second
                                                                      : VocableProgress{},
                                                              folly::sorted_vector_set<std::size_t>{card_index},
                                                              dicItemVec);

            cardMeta.vocableIndices_insert(vocable_index);
        }
    }
}

auto WalkableData::getVocableIdsInOrder(const CardDB::CardPtr& card,
                                        const std::map<unsigned, unsigned>& vocableChoices)
        -> std::vector<VocableId>
{
    std::map<std::string, uint> zhdic_vocableMeta;
    const ZH_Annotator& annotator = card->getAnnotator();
    std::vector<VocableId> vocableIds;
    ranges::transform(annotator.Items() | std::views::filter([](const ZH_Annotator::Item& item) {
                          return not item.dicItemVec.empty();
                      }),
                      std::back_inserter(vocableIds),
                      [&vocableChoices](const ZH_Annotator::Item& item) -> VocableId {
                          // TODO remove static_cast
                          auto vocId = static_cast<VocableId>(item.dicItemVec.front().id);
                          if (const auto it = vocableChoices.find(vocId);
                              it != vocableChoices.end()) {
                              // TODO remove static_cast
                              vocId = static_cast<VocableId>(it->second);
                          }
                          return vocId;
                      });
    return vocableIds;
}

auto WalkableData::generateVocableIdProgressMap() const -> std::map<VocableId, VocableProgress>
{
    std::map<VocableId, VocableProgress> id_progress;
    ranges::transform(vocables.id_index_view(),
                      std::inserter(id_progress, id_progress.begin()),
                      [this](const auto& id_index) -> std::pair<VocableId, VocableProgress> {
                          const auto [vocableId, index] = id_index;
                          return {vocableId, vocables[index].Progress()};
                      });
    return id_progress;
}

void WalkableData::saveProgress() const
{
    spdlog::info("Saving Progress..");
    db.SaveProgressVocables(generateVocableIdProgressMap());
}
} // namespace sr
