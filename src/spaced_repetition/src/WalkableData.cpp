#include <WalkableData.h>
#include <bits/ranges_algo.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <boost/range/combine.hpp>
#include <ranges>

namespace ranges = std::ranges;
namespace views = std::views;

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

auto VocableMeta::CardIndices() const -> folly::sorted_vector_set<std::size_t>
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

auto CardMeta::VocableIndices() const -> folly::sorted_vector_set<std::size_t>
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

auto WalkableData::Vocables() const -> utl::index_map<VocableMeta>
{
    return vocables;
}

auto WalkableData::Cards() const -> utl::index_map<CardMeta>
{
    return cards;
}

auto WalkableData::timingAndNVocables(
        const CardMeta& card,
        const folly::sorted_vector_set<std::size_t>& deadVocables) const -> TimingAndNVocables
{
    folly::sorted_vector_set<std::size_t> lifeVocables;
    ranges::set_difference(card.VocableIndices(), deadVocables, std::inserter(lifeVocables, lifeVocables.begin()));
    // auto progress = lifeVocables | views::transform([this](std::size_t vocableIndex) { return vocables[vocableIndex].Progress(); });
    auto progress = lifeVocables | views::transform(&vocables.operator[]);
    return {0, 0};
}
auto WalkableData::timingAndNVocables(const CardMeta& card) const -> TimingAndNVocables
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
                                                              folly::sorted_vector_set<std::size_t>{card->Id()},
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
