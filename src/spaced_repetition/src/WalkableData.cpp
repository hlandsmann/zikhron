#include <WalkableData.h>
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

CardMeta::CardMeta(CardProgress _progress, folly::sorted_vector_set<std::size_t> _cardIndices)
    : progress{std::move(_progress)}
    , vocableIndices{std::move(_cardIndices)} {}

WalkableData::WalkableData(std::shared_ptr<zikhron::Config> config)
    : db{std::move(config)}
{
    fillIndexMaps();
}

void WalkableData::fillIndexMaps()
{
    for (const auto& [_, card] : db.getCards() /* | views::take(10) */) {
        insertVocabularyOfCard(card);
    }
    spdlog::info("number of vocables: {}", vocables.size());
    spdlog::info("number of cards: {}", cards.size());
    for(const auto& voc:vocables){
      spdlog::info("card num: {}", voc.cardIndices.size());
    }
    std::for_each(vocables.cbegin(), vocables.cend(), [](const auto& voc) {
        spdlog::info("card num: {}", voc.cardIndices.size());
    });
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
            vocable.cardIndices.insert(card_index);
            cardMeta.vocableIndices.insert(*optionalIndex);
        } else {
            const auto& progressVocables = db.ProgressVocables();
            auto itVoc = progressVocables.find(vocId);
            const auto& [vocable_index, _] = vocables.emplace(vocId,
                                                              (itVoc != progressVocables.end())
                                                                      ? itVoc->second
                                                                      : VocableProgress{},
                                                              folly::sorted_vector_set<std::size_t>{card->Id()},
                                                              dicItemVec);

            cardMeta.vocableIndices.insert(vocable_index);
        }
    }
    // spdlog::info("voc: {}, dic: {}", vocableIds.size(), annotatorItems.size());
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
