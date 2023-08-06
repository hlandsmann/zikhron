#pragma once

#include "CardProgress.h"
#include "VocableProgress.h"

#include <annotation/TextCard.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Config.h>
#include <utils/StringU8.h>

#include <filesystem>
#include <memory>
#include <nlohmann/json_fwd.hpp>

class DataBase
{
    static constexpr std::string_view s_content = "content";

    static constexpr std::string_view s_path_meta = "/home/harmen/zikhron";
    static constexpr std::string_view s_fn_metaVocableSR = "metaVocableSR.json";
    static constexpr std::string_view s_fn_metaCardSR = "metaCardSR.json";
    static constexpr std::string_view s_fn_annotationChoices = "annotationChoices.json";
    static constexpr std::string_view s_fn_vocableChoices = "vocableChoices.json";

public:
    DataBase(std::shared_ptr<zikhron::Config> config);
    [[nodiscard]] auto VocableChoices() const -> const std::map<unsigned, unsigned>&;
    [[nodiscard]] auto ProgressVocables() const -> const std::map<unsigned, VocableProgress>&;
    [[nodiscard]] auto ProgressCards() const -> const std::map<unsigned, CardProgress>&;
    [[nodiscard]] auto getCards() const -> const std::map<unsigned, CardDB::CardPtr>&;

private:
    std::shared_ptr<zikhron::Config> config;

    std::shared_ptr<const ZH_Dictionary> zhDictionary;

    using CharacterSequence = std::vector<utl::CharU8>;
    using Combination = std::vector<int>;
    using AnnotationChoiceMap = std::map<CharacterSequence, Combination>;
    AnnotationChoiceMap annotationChoices;
    std::map<unsigned, unsigned> vocableChoices;
    std::shared_ptr<CardDB> cardDB;
    std::map<unsigned, VocableProgress> progressVocables;
    std::map<unsigned, CardProgress> progressCards;

    static void saveJsonToFile(const std::filesystem::path& fn, const nlohmann::json& js);
    static auto loadJsonFromFile(const std::filesystem::path& fn) -> nlohmann::json;
    static auto loadAnnotationChoices(const std::filesystem::path& annotationChoicesPath) -> AnnotationChoiceMap;
    static auto loadVocableChoices(const std::filesystem::path& vocableChoicesPath) -> std::map<unsigned, unsigned>;
    // static auto getVocableIdsInOrder(const CardDB::CardPtr& card,
    //                                  std::map<unsigned, unsigned> vocableChoices) -> std::vector<uint>;

    template<class mapped_value>
    static auto jsonToMap(const nlohmann::json& jsonMeta) -> std::map<unsigned, mapped_value>;
    static auto loadProgressVocables(
            const std::filesystem::path& progressVocablePath) -> std::map<unsigned, VocableProgress>;
    static auto loadProgressCards(
            const std::filesystem::path& progressCardsPath) -> std::map<unsigned, CardProgress>;
};
