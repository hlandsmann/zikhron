#include <WalkableData.h>
#include <boost/range/combine.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <ranges>

namespace ranges = std::ranges;
namespace views = std::views;

WalkableData::WalkableData(std::shared_ptr<zikhron::Config> config)
    : db{std::move(config)}
{
    fillIndexMaps();
}

void WalkableData::fillIndexMaps()
{
    for (const auto& [_, card] : db.getCards()) {
        insertVocabularyOfCard(card);
    }
}

void WalkableData::insertVocabularyOfCard(const CardDB::CardPtr& card)
{
    const ZH_Annotator& annotator = card->getAnnotator();

    // Its unfortunate, that we cannot siDictionarymply use a view.... but we gotta live with that.
    // So lets create a temporary vector annotatorItems to represent that view.
    std::vector<std::reference_wrapper<const ZH_Annotator::ZH_dicItemVec>> annotatorItems;
    ranges::transform(annotator.Items() | views::filter([](const ZH_Annotator::Item& item) {
                          return not item.dicItemVec.empty();
                      }),
                      std::back_inserter(annotatorItems),
                      [](const auto& item) -> std::reference_wrapper<const ZH_Annotator::ZH_dicItemVec> {
                          return item.dicItemVec;
                      });

    std::vector<uint> vocableIds = getVocableIdsInOrder(card, db.VocableChoices());
    for (const auto& [vocId, dicItemVec] : boost::combine(vocableIds, annotatorItems)) {
        const auto& dicEntry = db.Dictionary()->EntryFromPosition(vocId, CharacterSetType::Simplified);
        const auto& key = dicEntry.key;
        //
        // if (auto it = zhdic_vocableMeta.find(key); it != zhdic_vocableMeta.end()) {
        //     auto& vocableMeta = id_vocableMeta[vocId];
        //     vocableMeta.cardIds.insert(cardId);
        //     cm.vocableIds.insert(vocId);
        // } else {
        //     id_vocableMeta[vocId] = {.cardIds = {cardId}};
        //     zhdic_vocableMeta[key] = vocId;
        //     id_vocable[vocId] = ZH_dicItemVec{dicItemVec};
        //     cm.vocableIds.insert(vocId);
        // }
    }
    // cm.cardId = cardId;
}

auto WalkableData::getVocableIdsInOrder(const CardDB::CardPtr& card,
                                    const std::map<unsigned, unsigned>& vocableChoices) -> std::vector<uint>
{
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
