#pragma once
#include "CardMeta.h"
#include "CardProgress.h"
#include "VocableMeta.h"
#include "VocableProgress.h"
#include "srtypes.h"

#include <annotation/CardDB.h>
#include <annotation/Ease.h>
#include <annotation/ZH_Tokenizer.h>
#include <dictionary/ZH_Dictionary.h>
#include <folly/sorted_vector_types.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <utils/StringU8.h>
#include <utils/index_map.h>
#include <utils/min_element_val.h>

#include <filesystem>
#include <map>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <string_view>
#include <vector>

#include <sys/types.h>

namespace sr {
class DataBase
{
    static constexpr std::string_view s_content = "content";

    static constexpr std::string_view s_fn_metaVocableSR = "metaVocableSR.json";
    static constexpr std::string_view s_fn_metaCardSR = "metaCardSR.json";
    static constexpr std::string_view s_fn_annotationChoices = "annotationChoices.json";
    static constexpr std::string_view s_fn_vocableChoices = "vocableChoices.json";

public:
    using CharacterSequence = Card::CharacterSequence;
    using Combination = Card::Combination;
    using AnnotationChoiceMap = Card::AnnotationChoiceMap;

    DataBase(std::shared_ptr<zikhron::Config> config);
    [[nodiscard]] auto Dictionary() const -> std::shared_ptr<const ZH_Dictionary>;
    [[nodiscard]] auto Vocables() const -> const utl::index_map<VocableId, VocableMeta>&;
    [[nodiscard]] auto Cards() -> utl::index_map<CardId, CardMeta>&;

    void setEaseVocable(VocableId, const Ease&);
    void triggerVocable(VocableId, CardId);
    void resetCardsContainingVocable(VocableId vocId);
    void saveProgress() const;
    void addVocableChoice(VocableId oldVocId, VocableId newVocId);
    void addAnnotation(const ZH_Tokenizer::Combination& combination,
                       const std::vector<utl::CharU8>& characterSequence);

    [[nodiscard]] auto unmapVocableChoice(VocableId) const -> VocableId;

private:
    template<class key_type, class mapped_value>
    static auto jsonToMap(const nlohmann::json& jsonMeta) -> std::map<key_type, mapped_value>;
    static void saveJsonToFile(const std::filesystem::path& fn, const nlohmann::json& js);
    static auto loadJsonFromFile(const std::filesystem::path& fn) -> nlohmann::json;

    static auto loadVocableChoices(const std::filesystem::path& vocableChoicesPath) -> vocId_vocId_map;
    static auto loadAnnotationChoices(
            const std::filesystem::path& annotationChoicesPath) -> std::shared_ptr<AnnotationChoiceMap>;
    static auto loadProgressVocables(
            const std::filesystem::path& progressVocablePath) -> std::map<VocableId, VocableProgress>;
    static auto loadProgressCards(
            const std::filesystem::path& progressCardsPath) -> std::map<CardId, CardProgress>;
    void saveAnnotationChoices() const;
    void saveVocableChoices() const;
    void saveProgressVocables() const;

    [[nodiscard]] auto generateVocableIdProgressMap() const -> std::map<VocableId, VocableProgress>;
    void fillIndexMaps();
    void addNewVocableIds(const vocId_set& newVocableIds);

    std::shared_ptr<zikhron::Config> config;

    std::shared_ptr<const ZH_Dictionary> zhDictionary;

    std::shared_ptr<AnnotationChoiceMap> annotationChoices;
    vocId_vocId_map vocableChoices;
    std::shared_ptr<CardDB> cardDB;
    std::map<VocableId, VocableProgress> progressVocables;
    std::map<CardId, CardProgress> progressCards;

    std::shared_ptr<utl::index_map<VocableId, VocableMeta>> vocables;
    std::shared_ptr<utl::index_map<CardId, CardMeta>> cards;
};
} // namespace sr
