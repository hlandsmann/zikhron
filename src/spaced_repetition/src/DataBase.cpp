#include <CardMeta.h>
#include <CardProgress.h>
#include <DataBase.h>
#include <VocableMeta.h>
#include <VocableProgress.h>
#include <annotation/CardDB.h>
#include <annotation/Ease.h>
#include <annotation/ZH_Tokenizer.h>
#include <dictionary/ZH_Dictionary.h>
#include <fmt/format.h>
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

auto DataBase::Vocables() const -> const utl::index_map<VocableId, VocableMeta>&
{
    return *vocables;
}

auto DataBase::Cards() -> utl::index_map<CardId, CardMeta>&
{
    return *cards;
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

void DataBase::saveProgress() const
{
    spdlog::info("Saving Progress..");
    saveProgressVocables();
    saveAnnotationChoices();
    saveVocableChoices();
}

void DataBase::addVocableChoice(VocableId oldVocId, VocableId newVocId)
{
    vocableChoices[oldVocId] = newVocId;
    for (auto& cardMeta : *cards) {
        cardMeta.addVocableChoice(oldVocId, newVocId);
    }
}

void DataBase::addAnnotation(const ZH_Tokenizer::Combination& combination,
                             const std::vector<utl::CharU8>& characterSequence)
{
    (*annotationChoices)[characterSequence] = combination;

    std::set<uint> cardsWithCharSeq;

    for (const auto& [id, cardPtr] : cardDB->get()) {
        if (cardPtr->getTokenizer().ContainsCharacterSequence(characterSequence)) {
            cardsWithCharSeq.insert(id);
            auto& cardMeta = cards->at_id(id).second;
            cardMeta.resetAnnotation();
            addNewVocableIds(cardMeta.NewVocableIds());
        }
    }
    spdlog::debug("relevant cardIds: {}", fmt::join(cardsWithCharSeq, ","));
}

auto DataBase::unmapVocableChoice(VocableId vocableId) const -> VocableId
{
    if (auto it = ranges::find_if(vocableChoices,
                                  [vocableId](const std::pair<VocableId, VocableId> mapping) {
                                      return mapping.second == vocableId;
                                  });
        it != vocableChoices.end()) {
        return it->first;
    }
    return vocableId;
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

void DataBase::saveJsonToFile(const std::filesystem::path& fn, const nlohmann::json& js)
{
    std::ofstream ofs(fn);
    ofs << js.dump(4);
}

auto DataBase::loadJsonFromFile(const std::filesystem::path& fn) -> nlohmann::json
{
    std::ifstream ifs(fn);
    return nlohmann::json::parse(ifs);
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

auto DataBase::loadAnnotationChoices(
        const std::filesystem::path& annotationChoicesPath) -> std::shared_ptr<AnnotationChoiceMap>
{
    try {
        auto annotationChoices = std::make_shared<AnnotationChoiceMap>();
        nlohmann::json choicesJson = loadJsonFromFile(annotationChoicesPath);
        ranges::transform(choicesJson,
                          std::inserter(*annotationChoices, annotationChoices->begin()),
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

void DataBase::saveAnnotationChoices() const
{
    try {
        nlohmann::json array = nlohmann::json::array();
        ranges::transform(*annotationChoices,
                          std::back_inserter(array),
                          [](const std::pair<CharacterSequence, Combination>& choice) -> nlohmann::json {
                              return {{"char_seq", choice.first}, {"combination", choice.second}};
                          });
        saveJsonToFile(config->DatabaseDirectory() / s_fn_annotationChoices, array);
    } catch (const std::exception& e) {
        spdlog::error("Saving annotation choices failed with Error: {}", e.what());
    }
}

void DataBase::saveVocableChoices() const
{
    try {
        nlohmann::json array = nlohmann::json::array();
        ranges::transform(vocableChoices,
                          std::back_inserter(array),
                          [](const std::pair<uint, uint>& choice) -> nlohmann::json {
                              return {{"id", choice.first}, {"map_id", choice.second}};
                          });
        saveJsonToFile(config->DatabaseDirectory() / s_fn_vocableChoices, array);
    } catch (const std::exception& e) {
        spdlog::error("Saving vocable choices failed with Error: {}", e.what());
    }
}

void DataBase::saveProgressVocables() const
{
    std::map<VocableId, VocableProgress> id_progress = generateVocableIdProgressMap();
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

void DataBase::fillIndexMaps()
{
    folly::sorted_vector_set<VocableId> allVocableIds;
    for (const auto& [id, cardPtr] : cardDB->get()) {
        (*cards).emplace(id, cardPtr, vocables, vocableChoices);
    }

    for (const auto& card : *cards) {
        const auto& vocableIds = card.VocableIds();
        allVocableIds.insert(vocableIds.begin(), vocableIds.end());
    }
    for (VocableId vocId : allVocableIds) {
        if (progressVocables.contains(vocId)) {
            vocables->emplace(vocId, progressVocables.at(vocId));
        } else {
            vocables->emplace(vocId, VocableProgress::new_vocable);
        }
    }
    for (const auto& [cardIndex, cardMeta] : views::enumerate(cards->vspan())) {
        for (const auto& vocableIndex : cardMeta.VocableIndices()) {
            (*vocables)[vocableIndex].cardIndices_insert(static_cast<std::size_t>(cardIndex));
        }
    }
    spdlog::info("number of vocables: {}", allVocableIds.size());
    spdlog::info("number of cards: {}", cards->size());
}

void DataBase::addNewVocableIds(const vocId_set& newVocableIds)
{
    for (VocableId newVocId : newVocableIds) {
        vocables->emplace(newVocId, VocableProgress::new_vocable);
    }
}

} // namespace sr
