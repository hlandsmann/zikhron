#include <CardMeta.h>
#include <CardProgress.h>
#include <DataBase.h>
#include <VocableMeta.h>
#include <VocableProgress.h>
#include <annotation/CardDB.h>
#include <annotation/Ease.h>
#include <annotation/ZH_Annotator.h>
#include <dictionary/ZH_Dictionary.h>
#include <folly/sorted_vector_types.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <srtypes.h>
#include <utils/StringU8.h>
#include <utils/index_map.h>
#include <utils/min_element_val.h>

#include <algorithm>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <ranges>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <sys/types.h>

namespace ranges = std::ranges;
namespace views = std::views;

namespace sr {
DataBase::DataBase(std::shared_ptr<zikhron::Config> _config)
    : config{std::move(_config)}
    , zhDictionary{std::make_shared<ZH_Dictionary>(config->Dictionary())}
    , annotationChoices{loadAnnotationChoices(config->DatabaseDirectory() / s_fn_annotationChoices)}
    , vocableChoices{loadVocableChoices(config->DatabaseDirectory() / s_fn_vocableChoices)}
    , cardDB{std::make_shared<CardDB>(config->DatabaseDirectory() / CardDB::s_cardSubdirectory,
                                      zhDictionary,
                                      annotationChoices)}
    , progressVocables{loadProgressVocables(config->DatabaseDirectory() / s_fn_metaVocableSR)}
    , progressCards{loadProgressCards(config->DatabaseDirectory() / s_fn_metaCardSR)}
    , vocables{std::make_shared<utl::index_map<VocableId, VocableMeta>>()}
    , cards{std::make_shared<utl::index_map<CardId, CardMeta>>()}
{
    fillIndexMaps();
}

auto DataBase::Dictionary() const -> std::shared_ptr<const ZH_Dictionary>
{
    return zhDictionary;
}

// auto DataBase::AnnotationChoices() const -> const AnnotationChoiceMap&
// {
//     return annotationChoices;
// }

auto DataBase::VocableChoices() const -> const vocId_vocId_map&
{
    return vocableChoices;
}

auto DataBase::ProgressVocables() const -> const std::map<VocableId, VocableProgress>&
{
    return progressVocables;
}

auto DataBase::ProgressCards() const -> const std::map<CardId, CardProgress>&
{
    return progressCards;
}

auto DataBase::getCards() const -> const std::map<CardId, CardDB::CardPtr>&
{
    return cardDB->get();
}

auto DataBase::loadJsonFromFile(const std::filesystem::path& fn) -> nlohmann::json
{
    std::ifstream ifs(fn);
    return nlohmann::json::parse(ifs);
}

void DataBase::saveJsonToFile(const std::filesystem::path& fn, const nlohmann::json& js)
{
    std::ofstream ofs(fn);
    ofs << js.dump(4);
}

auto DataBase::loadAnnotationChoices(const std::filesystem::path& annotationChoicesPath) -> AnnotationChoiceMap
{
    try {
        AnnotationChoiceMap annotationChoices;
        nlohmann::json choicesJson = loadJsonFromFile(annotationChoicesPath);
        ranges::transform(choicesJson,
                          std::inserter(annotationChoices, annotationChoices.begin()),
                          [](const nlohmann::json& choice) -> std::pair<CharacterSequence, Combination> {
                              nlohmann::json char_seqJson = choice["char_seq"];
                              nlohmann::json combinationJson = choice["combination"];
                              CharacterSequence char_seq;
                              Combination combination;
                              ranges::transform(char_seqJson,
                                                std::back_inserter(char_seq),
                                                [](const nlohmann::json& character) -> utl::CharU8 {
                                                    return {std::string(character)};
                                                });
                              ranges::transform(combinationJson,
                                                std::back_inserter(combination),
                                                [](const nlohmann::json& c) -> int { return c; });
                              return {char_seq, combination};
                          });
        return annotationChoices;
    } catch (const std::exception& e) {
        spdlog::error("Load of AnnotationChoice file failed, Error: {}", e.what());
        return {};
    }
}

auto DataBase::loadVocableChoices(const std::filesystem::path& vocableChoicesPath) -> vocId_vocId_map
{
    vocId_vocId_map vocableChoices;
    try {
        nlohmann::json choicesJson = loadJsonFromFile(vocableChoicesPath);
        ranges::transform(choicesJson,
                          std::inserter(vocableChoices, vocableChoices.begin()),
                          [](const nlohmann::json& choice) -> std::pair<VocableId, VocableId> {
                              nlohmann::json id = choice["id"];
                              nlohmann::json map_id = choice["map_id"];

                              return {id, map_id};
                          });
    } catch (const std::exception& e) {
        spdlog::error("Load of vocable choice file failed, Error: {}", e.what());
    }
    return vocableChoices;
}

template<class key_type, class mapped_value>
auto DataBase::jsonToMap(const nlohmann::json& jsonMeta) -> std::map<key_type, mapped_value>
{
    std::map<key_type, mapped_value> map;
    const auto& content = jsonMeta.at(std::string(s_content));
    using sr_t = typename std::decay_t<decltype(map)>::mapped_type;
    ranges::transform(content, std::inserter(map, map.begin()), &sr_t::fromJson);
    return map;
}

auto DataBase::loadProgressVocables(
        const std::filesystem::path& progressVocablePath) -> std::map<VocableId, VocableProgress>
{
    std::map<VocableId, VocableProgress> id_progress;
    try {
        nlohmann::json jsonVocable = loadJsonFromFile(progressVocablePath);
        id_progress = jsonToMap<VocableId, VocableProgress>(jsonVocable);
        spdlog::debug("Vocable SR file {} loaded!", s_fn_metaVocableSR);
    } catch (const std::exception& e) {
        spdlog::error("Vocabulary SR load for {} failed! Exception {}", progressVocablePath.c_str(), e.what());
    }
    return id_progress;
}

void DataBase::SaveProgressVocables(std::map<VocableId, VocableProgress> id_progress) const
{
    auto generateJsonFromMap = [](const auto& map) -> nlohmann::json {
        nlohmann::json jsonMeta = nlohmann::json::object();
        auto& content = jsonMeta[std::string(s_content)];
        content = nlohmann::json::array();

        using sr_t = typename std::decay_t<decltype(map)>::mapped_type;
        ranges::transform(map, std::back_inserter(content), &sr_t::toJson);
        return jsonMeta;
    };
    std::filesystem::create_directory(config->DatabaseDirectory());

    try {
        nlohmann::json jsonVocSR = generateJsonFromMap(id_progress);
        saveJsonToFile(config->DatabaseDirectory() / s_fn_metaVocableSR, jsonVocSR);
    } catch (const std::exception& e) {
        spdlog::error("Saving of vocable progress failed. Error: {}", e.what());
    }
}

auto DataBase::loadProgressCards(
        const std::filesystem::path& progressCardsPath) -> std::map<CardId, CardProgress>
{
    std::map<CardId, CardProgress> id_card;
    try {
        nlohmann::json jsonCardSR = loadJsonFromFile(progressCardsPath);
        id_card = jsonToMap<CardId, CardProgress>(jsonCardSR);
        spdlog::debug("Card SR file {} loaded!", s_fn_metaCardSR);
    } catch (const std::exception& e) {
        spdlog::error("Card SR load for {} failed! Exception {}", progressCardsPath.c_str(), e.what());
    }
    return id_card;
}

// TODO OLD CODE //////////////////////////////////////////////////////////////////////////////////////////

auto DataBase::Vocables() const -> const utl::index_map<VocableId, VocableMeta>&
{
    return *vocables;
}

auto DataBase::Cards() -> utl::index_map<CardId, CardMeta>&
{
    return *cards;
}

auto DataBase::getCardCopy(size_t cardIndex) const -> CardDB::CardPtr
{
    CardId cardId = cards->id_from_index(cardIndex);
    const CardDB::CardPtr& cardPtr = getCards().at(cardId);
    return cardPtr->clone();
}

auto DataBase::getVocableIdsInOrder(size_t cardIndex) const -> std::vector<VocableId>
{
    CardId cardId = cards->id_from_index(cardIndex);
    const auto& cardPtr = cardDB->atId(cardId);
    // const auto& vocableChoices = VocableChoices();
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

auto DataBase::getActiveVocables(size_t cardIndex) -> std::set<VocableId>
{
    const auto& activeVocableIndices = (*cards)[cardIndex].getTimingAndVocables(true).vocables;
    std::set<VocableId> activeVocableIds;
    ranges::transform(activeVocableIndices, std::inserter(activeVocableIds, activeVocableIds.begin()),
                      [this](size_t vocableIndex) -> VocableId {
                          return vocables->id_from_index(vocableIndex);
                      });

    return activeVocableIds;
}

auto DataBase::getRelevantEase(size_t cardIndex) -> std::map<VocableId, Ease>
{
    std::set<VocableId> activeVocables = getActiveVocables(cardIndex);
    std::map<VocableId, Ease> ease;
    ranges::transform(
            activeVocables,
            std::inserter(ease, ease.begin()),
            [&, this](VocableId vocId) -> std::pair<VocableId, Ease> {
                const VocableProgress& vocSR = vocables->at_id(vocId).second.Progress();
                // const VocableProgress vocSR = id_vocableSR.contains(vocId) ? id_vocableSR.at(vocId) : VocableProgress{};
                spdlog::debug("Easefactor of {} is {:.2f}, invervalDay {:.2f} - id: {}",
                              zhDictionary->EntryFromPosition(vocId, CharacterSetType::Simplified).key,
                              vocSR.EaseFactor(),
                              vocSR.IntervalDay(),
                              vocId);
                return {vocId, {vocSR.IntervalDay(), vocSR.dueDays(), vocSR.EaseFactor()}};
            });
    return ease;
}

void DataBase::setEaseVocable(VocableId vocId, const Ease& ease)
{
    VocableMeta& vocable = vocables->at_id(vocId).second;
    vocable.advanceByEase(ease);
}

void DataBase::triggerVocable(VocableId vocId, CardId cardId)
{
    VocableMeta& vocable = vocables->at_id(vocId).second;
    vocable.triggerByCardId(cardId);
}

void DataBase::resetCardsContainingVocable(VocableId vocId)
{
    const auto& [_, vocable] = vocables->at_id(vocId);
    const auto& cardIndices = vocable.CardIndices();
    for (size_t card_index : cardIndices) {
        auto& card = (*cards)[card_index];
        card.resetTimingAndVocables();
    }
}

void DataBase::fillIndexMaps()
{
    folly::sorted_vector_set<VocableId> allVocableIds;
    for (const auto& [id, cardPtr] : getCards()) {
        (*cards).emplace(id, cardPtr, vocables, vocableChoices);
        // insertVocabularyOfCard(card);
    }

    for (const auto& card : *cards) {
        const auto& vocableIds = card.VocableIds();
        allVocableIds.insert(vocableIds.begin(), vocableIds.end());
    }
    // const auto& progressVocables = ProgressVocables();

    // size_t found = 0;
    // size_t notFound = 0;
    for (VocableId vocId : allVocableIds) {
        VocableId tmpVocId = vocId;
        if (vocableChoices.contains(vocId)) {
            tmpVocId = vocableChoices[vocId];
        }
        // if (progressVocables.contains(tmpVocId)) {
        //     found++;
        // } else {
        //     notFound++;
        // }
        vocables->emplace(vocId, progressVocables.at(tmpVocId));
    }
    for (const auto& [cardIndex, cardMeta] : views::enumerate(cards->vspan())) {
        for (const auto& vocableIndex : cardMeta.VocableIndices()) {
            (*vocables)[vocableIndex].cardIndices_insert(static_cast<std::size_t>(cardIndex));
        }
    }
    // spdlog::info("found: {}, notFound: {}", found, notFound);
    // spdlog::info("number of vocables: {}", vocables->size());
    spdlog::info("number of vocables: {}", allVocableIds.size());
    spdlog::info("number of cards: {}", cards->size());
}

void DataBase::insertVocabularyOfCard(const CardDB::CardPtr& card)
{
    // const ZH_Annotator& annotator = card->getAnnotator();
    // std::map<std::string, uint> zhdic_vocableMeta;
    // // Its unfortunate, that we cannot simply use a view.... but we gotta live with that.
    // // So lets create a temporary vector annotatorItems to represent that view.
    // std::vector<std::reference_wrapper<const ZH_Annotator::ZH_dicItemVec>> annotatorItems;
    // ranges::transform(annotator.Items() | views::filter([](const ZH_Annotator::Item& item) {
    //                       return not item.dicItemVec.empty();
    //                   }),
    //                   std::back_inserter(annotatorItems),
    //                   [](const auto& item) -> std::reference_wrapper<const ZH_Annotator::ZH_dicItemVec> {
    //                       return item.dicItemVec;
    //                   });
    //
    // // TODO remove static cast
    // auto [card_index, cardMetaRef] = cards->emplace(static_cast<CardId>(card->Id()),
    //                                                 CardMeta{card,
    //                                                          folly::sorted_vector_set<std::size_t>{},
    //                                                          vocables});
    // auto& cardMeta = cardMetaRef.get();
    // std::vector<VocableId> vocableIds = getVocableIdsInOrder(card, vocableChoices);
    // for (const auto& [vocId, dicItemVec] : views::zip(vocableIds, annotatorItems)) {
    //     const auto& optionalIndex = vocables->optional_index(vocId);
    //     if (optionalIndex.has_value()) {
    //         auto& vocable = (*vocables)[*optionalIndex];
    //         vocable.cardIndices_insert(card_index);
    //         cardMeta.vocableIndices_insert(*optionalIndex);
    //     } else {
    //         const auto& progressVocables = ProgressVocables();
    //         auto itVoc = progressVocables.find(vocId);
    //         const auto& [vocable_index, _] = vocables->emplace(vocId,
    //                                                            (itVoc != progressVocables.end())
    //                                                                    ? itVoc->second
    //                                                                    : VocableProgress{},
    //                                                            folly::sorted_vector_set<std::size_t>{card_index},
    //                                                            dicItemVec);
    //
    //         cardMeta.vocableIndices_insert(vocable_index);
    //     }
    // }
}

auto DataBase::generateVocableIdProgressMap() const -> std::map<VocableId, VocableProgress>
{
    std::map<VocableId, VocableProgress> id_progress;
    ranges::transform(vocables->id_index_view(),
                      std::inserter(id_progress, id_progress.begin()),
                      [this](const auto& id_index) -> std::pair<VocableId, VocableProgress> {
                          const auto [vocableId, index] = id_index;
                          return {vocableId, (*vocables)[index].Progress()};
                      });
    return id_progress;
}

void DataBase::saveProgress() const
{
    spdlog::info("Saving Progress..");
    // SaveProgressVocables(generateVocableIdProgressMap());
}

void DataBase::addVocableChoice(uint vocId, uint vocIdOldChoice, uint vocIdNewChoice)
{
    if (vocIdOldChoice == vocIdNewChoice) {
        return;
    }

    // if (vocId != vocIdNewChoice)
    //     id_id_vocableChoices[vocId] = vocIdNewChoice;
    // else
    //     id_id_vocableChoices.erase(vocId);
}
} // namespace sr
