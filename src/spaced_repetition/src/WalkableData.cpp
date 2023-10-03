#include "VocableProgress.h"

#include <CardProgress.h>
#include <WalkableData.h>
#include <annotation/Card.h>
#include <annotation/ZH_Annotator.h>
#include <bits/ranges_algo.h>
#include <folly/sorted_vector_types.h>
#include <misc/Config.h>
#include <spdlog/spdlog.h>
#include <utils/index_map.h>
#include <utils/min_element_val.h>

#include <algorithm>
#include <boost/range/combine.hpp>
#include <cstddef>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <ranges>
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

auto VocableMeta::Progress() const -> VocableProgress
{
    return progress;
}

auto VocableMeta::CardIndices() const -> const folly::sorted_vector_set<std::size_t>&
{
    return cardIndices;
}

void VocableMeta::cardIndices_insert(std::size_t cardIndex)
{
    cardIndices.insert(cardIndex);
}

CardMeta::CardMeta(CardProgress _progress, folly::sorted_vector_set<std::size_t> _vocableIndices)
    : progress{std::move(_progress)}
    , vocableIndices{std::move(_vocableIndices)} {}

auto CardMeta::Progress() const -> CardProgress
{
    return progress;
}

auto CardMeta::VocableIndices() const -> const folly::sorted_vector_set<std::size_t>&
{
    return vocableIndices;
}

void CardMeta::vocableIndices_insert(std::size_t vocableIndex)
{
    vocableIndices.insert(vocableIndex);
}

WalkableData::WalkableData(std::shared_ptr<zikhron::Config> config)
    : db{std::move(config)}
{
    fillIndexMaps();
}

auto WalkableData::Vocables() const -> const utl::index_map<VocableMeta>&
{
    return vocables;
}

auto WalkableData::Cards() const -> const utl::index_map<CardMeta>&
{
    return cards;
}

auto WalkableData::timingAndNVocables(
        const CardMeta& card,
        const folly::sorted_vector_set<std::size_t>& deadVocables) const -> TimingAndVocables
{
    folly::sorted_vector_set<std::size_t> lifeVocables;
    ranges::set_difference(card.VocableIndices(), deadVocables, std::inserter(lifeVocables, lifeVocables.begin()));
    if (lifeVocables.empty()) {
        return {};
    }
    auto vocindex_progresses = lifeVocables | views::transform(index_vocableProgress);
    // auto progresses = vocindex_progresses | views::values;
    auto progresses = lifeVocables | views::transform(vocable_progress);
    auto minIt = ranges::min_element(progresses, std::less{}, &VocableProgress::getRepeatRange);
    progresses.size();
    // auto minIt = ranges::min_element(lifeVocables | views::transform(vocable_progress), std::less{}, &VocableProgress::getRepeatRange);
    // auto [_, threshHoldProgress] = *minIt.base();
    auto threshHoldProgress = *minIt;
    // const auto& minr = threshHoldProgress.getRepeatRange();
    // spdlog::info("00000 --- min: {}, n: {}, max: {}",  minr.daysMin, minr.daysNormal, minr.daysMax);

    folly::sorted_vector_set<std::size_t> nextActiveVocables;
    ranges::copy_if(lifeVocables, std::inserter(nextActiveVocables, nextActiveVocables.begin()),
                    [&, this](const auto& vocableIndex) {
                        auto getRepeatRange = vocables[vocableIndex].Progress().getRepeatRange();
                        return threshHoldProgress.getRepeatRange().implies(getRepeatRange);
                    });

    auto progressesNextActive = nextActiveVocables | views::transform(vocable_progress);
    auto nextActiveIt = ranges::max_element(
            progressesNextActive,
            [](const auto& range1, const auto& range2) -> bool {
                // spdlog::info("min: {}, n: {}, max: {}", range1.daysMin, range1.daysNormal, range1.daysMax);
                return range1.daysMin < range2.daysMin;
            },
            &VocableProgress::getRepeatRange);
    const auto& nextActiveProgress = *nextActiveIt;
    // const auto& nar = nextActiveProgress.getRepeatRange();
    // spdlog::info("00000 --- min: {}, n: {}, max: {}",  nar.daysMin, nar.daysNormal, nar.daysMax);

    // std::vector<std::size_t> testv;
    // ranges::copy(nextActiveVocables, std::back_inserter(testv));
    // ranges::sort(testv, std::less{}, [this](std::size_t index) { return vocable_progress(index).getRepeatRange(); });
    // for (const auto& [index, prog] : testv | views::transform(index_vocableProgress)) {
    //     const auto& range = prog.getRepeatRange();
    //     spdlog::info("i: {:04d} --- min: {}, n: {}, max: {}", index, range.daysMin, range.daysNormal, range.daysMax);
    // }

    // auto minIt = utl::min_element_val(progresses, std::less{}, &VocableProgress::getRepeatRange);
    // auto x = minIt.getRepeatRange();
    // for (const auto& [vocableIndex, progress] : progresses) {
    // }
    return {.timing = nextActiveProgress.getRepeatRange().daysMin,
            .vocables = nextActiveVocables};
}

auto WalkableData::timingAndNVocables(size_t cardIndex) const -> TimingAndVocables
{
    const CardMeta& card = cards[cardIndex];
    return timingAndNVocables(card);
}

auto WalkableData::timingAndNVocables(const CardMeta& card) const -> TimingAndVocables
{
    return timingAndNVocables(card, {});
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

    const auto& progressCards = db.ProgressCards();
    auto itCard = progressCards.find(card->Id());
    auto [card_index, cardMetaRef] = cards.emplace(card->Id(),
                                                   (itCard != progressCards.end())
                                                           ? itCard->second
                                                           : CardProgress{},
                                                   folly::sorted_vector_set<std::size_t>{});
    auto& cardMeta = cardMetaRef.get();
    std::vector<uint> vocableIds = getVocableIdsInOrder(card, db.VocableChoices());
    for (const auto& [vocId, dicItemVec] : boost::combine(vocableIds, annotatorItems)) {
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
        -> std::vector<uint>
{
    std::map<std::string, uint> zhdic_vocableMeta;
    const ZH_Annotator& annotator = card->getAnnotator();
    std::vector<uint> vocableIds;
    ranges::transform(annotator.Items() | std::views::filter([](const ZH_Annotator::Item& item) {
                          return not item.dicItemVec.empty();
                      }),
                      std::back_inserter(vocableIds),
                      [&vocableChoices](const ZH_Annotator::Item& item) -> uint {
                          uint vocId = item.dicItemVec.front().id;
                          if (const auto it = vocableChoices.find(vocId);
                              it != vocableChoices.end()) {
                              vocId = it->second;
                          }
                          return vocId;
                      });
    return vocableIds;
}
} // namespace sr
