#include <DataBase.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <ranges>

namespace ranges = std::ranges;
namespace views = std::views;

DataBase::DataBase(std::shared_ptr<zikhron::Config> _config)
    : config{std::move(_config)}
    , zhDictionary{std::make_shared<ZH_Dictionary>(config->Dictionary())}
    , annotationChoices{loadAnnotationChoices(config->DatabaseDirectory() / s_fn_annotationChoices)}
    , vocableChoices{loadVocableChoices(config->DatabaseDirectory() / s_fn_vocableChoices)}
    , cardDB{std::make_shared<CardDB>(config->DatabaseDirectory() / CardDB::s_cardSubdirectory,
                                      zhDictionary,
                                      annotationChoices)}
{}

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

auto DataBase::loadVocableChoices(const std::filesystem::path& vocableChoicesPath) -> std::map<unsigned, unsigned>
{
    std::map<unsigned, unsigned> vocableChoices;
    try {
        nlohmann::json choicesJson = loadJsonFromFile(vocableChoicesPath);
        ranges::transform(choicesJson,
                          std::inserter(vocableChoices, vocableChoices.begin()),
                          [](const nlohmann::json& choice) -> std::pair<unsigned, unsigned> {
                              nlohmann::json id = choice["id"];
                              nlohmann::json map_id = choice["map_id"];

                              return {id, map_id};
                          });
    } catch (const std::exception& e) {
        spdlog::error("Load of vocable choice file failed, Error: {}", e.what());
    }
    return vocableChoices;
}

auto DataBase::createVocablesFromCardDB(
        const std::shared_ptr<CardDB>& cardDB) -> utl::index_map<Vocable>
{
    std::map<unsigned, Vocable> id_vocableSR;
    const std::map<unsigned, CardDB::CardPtr>& id_cards = cardDB->get();

    for (const auto& card : id_cards | views::values) {
    }
    ranges::transform(id_cards | views::values, std::inserter(id_vocableSR, id_vocableSR.begin()),
                      [](const auto& card) -> std::pair<unsigned, Vocable> {
                          const ZH_Annotator& annotator = card->getAnnotator();
                      });
}

auto DataBase::getVocableIdsInOrder(const CardDB::CardPtr& card,
                                    std::map<unsigned, unsigned> vocableChoices) -> std::vector<uint>
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

template<class mapped_value>
auto DataBase::jsonToMap(const nlohmann::json& jsonMeta) -> std::map<unsigned, mapped_value>
{
    std::map<unsigned, mapped_value> map;
    const auto& content = jsonMeta.at(std::string(s_content));
    using sr_t = typename std::decay_t<decltype(map)>::mapped_type;
    ranges::transform(content, std::inserter(map, map.begin()), &sr_t::fromJson);
    return map;
}

auto DataBase::loadProgressVocables(
        const std::filesystem::path& progressVocablePath) -> std::map<unsigned, Vocable>
{
    std::map<unsigned, Vocable> id_vocable;
    try {
        nlohmann::json jsonVocable = loadJsonFromFile(progressVocablePath);
        id_vocable = jsonToMap<Vocable>(jsonVocable);
        spdlog::debug("Vocable SR file {} loaded!", s_fn_metaVocableSR);
    } catch (const std::exception& e) {
        spdlog::error("Vocabulary SR load for {} failed! Exception {}", progressVocablePath.c_str(), e.what());
    }
    return id_vocable;
}

auto DataBase::loadProgressCards(
        const std::filesystem::path& progressCardsPath) -> std::map<unsigned, Card>
{
    std::map<unsigned, Card> id_card;
    try {
        nlohmann::json jsonCardSR = loadJsonFromFile(progressCardsPath);
        id_card = jsonToMap<Card>(jsonCardSR);
        spdlog::debug("Card SR file {} loaded!", s_fn_metaCardSR);
    } catch (const std::exception& e) {
        spdlog::error("Card SR load for {} failed! Exception {}", progressCardsPath.c_str(), e.what());
    }
    return id_card;
}
