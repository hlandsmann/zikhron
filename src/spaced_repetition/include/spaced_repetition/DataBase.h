#pragma once
#include "CardProgress.h"
#include "VocableProgress.h"

#include <annotation/Card.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <utils/StringU8.h>

#include <filesystem>
#include <map>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <string_view>
#include <vector>

class DataBase
{
    static constexpr std::string_view s_content = "content";

    static constexpr std::string_view s_path_meta = "/home/harmen/zikhron";
    static constexpr std::string_view s_fn_metaVocableSR = "metaVocableSR.json";
    static constexpr std::string_view s_fn_metaCardSR = "metaCardSR.json";
    static constexpr std::string_view s_fn_annotationChoices = "annotationChoices.json";
    static constexpr std::string_view s_fn_vocableChoices = "vocableChoices.json";

public:
    using CharacterSequence = std::vector<utl::CharU8>;
    using Combination = std::vector<int>;
    using AnnotationChoiceMap = std::map<CharacterSequence, Combination>;

    DataBase(std::shared_ptr<zikhron::Config> config);
    [[nodiscard]] auto Dictionary() const -> std::shared_ptr<const ZH_Dictionary>;
    // [[nodiscard]] auto AnnotationChoices() const -> const AnnotationChoiceMap&;
    [[nodiscard]] auto VocableChoices() const -> const std::map<unsigned, unsigned>&;
    [[nodiscard]] auto ProgressVocables() const -> const std::map<VocableId, VocableProgress>&;
    [[nodiscard]] auto ProgressCards() const -> const std::map<CardId, CardProgress>&;
    [[nodiscard]] auto getCards() const -> const std::map<unsigned, CardDB::CardPtr>&;

private:
    std::shared_ptr<zikhron::Config> config;

    std::shared_ptr<const ZH_Dictionary> zhDictionary;

    AnnotationChoiceMap annotationChoices;
    std::map<unsigned, unsigned> vocableChoices;
    std::shared_ptr<CardDB> cardDB;
    std::map<VocableId, VocableProgress> progressVocables;
    std::map<CardId, CardProgress> progressCards;

    static void saveJsonToFile(const std::filesystem::path& fn, const nlohmann::json& js);
    static auto loadJsonFromFile(const std::filesystem::path& fn) -> nlohmann::json;
    static auto loadAnnotationChoices(const std::filesystem::path& annotationChoicesPath) -> AnnotationChoiceMap;
    static auto loadVocableChoices(const std::filesystem::path& vocableChoicesPath) -> std::map<unsigned, unsigned>;

    template<class key_type, class mapped_value>
    static auto jsonToMap(const nlohmann::json& jsonMeta) -> std::map<key_type, mapped_value>;
    static auto loadProgressVocables(
            const std::filesystem::path& progressVocablePath) -> std::map<VocableId, VocableProgress>;
    static auto loadProgressCards(
            const std::filesystem::path& progressCardsPath) -> std::map<CardId, CardProgress>;
};
