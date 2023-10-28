#pragma once
#include "CardMeta.h"
#include "CardProgress.h"
#include "VocableMeta.h"
#include "VocableProgress.h"
#include "srtypes.h"

#include <annotation/CardDB.h>
#include <annotation/Ease.h>
#include <annotation/ZH_Annotator.h>
#include <dictionary/ZH_Dictionary.h>
#include <folly/sorted_vector_types.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <utils/StringU8.h>
#include <utils/index_map.h>
#include <utils/min_element_val.h>

#include <cstddef>
#include <filesystem>
#include <map>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <set>
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
    using CharacterSequence = std::vector<utl::CharU8>;
    using Combination = std::vector<int>;
    using AnnotationChoiceMap = std::map<CharacterSequence, Combination>;

    DataBase(std::shared_ptr<zikhron::Config> config);
    [[nodiscard]] auto Dictionary() const -> std::shared_ptr<const ZH_Dictionary>;
    // [[nodiscard]] auto AnnotationChoices() const -> const AnnotationChoiceMap&;
    [[nodiscard]] auto VocableChoices() const -> const vocId_vocId_map&;
    [[nodiscard]] auto ProgressVocables() const -> const std::map<VocableId, VocableProgress>&;
    [[nodiscard]] auto ProgressCards() const -> const std::map<CardId, CardProgress>&;
    [[nodiscard]] auto getCards() const -> const std::map<CardId, CardDB::CardPtr>&;
    void SaveProgressVocables(std::map<VocableId, VocableProgress> id_progress) const;
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    [[nodiscard]] auto Vocables() const -> const utl::index_map<VocableId, VocableMeta>&;
    [[nodiscard]] auto Cards() -> utl::index_map<CardId, CardMeta>&;
    [[nodiscard]] auto getCardCopy(size_t cardIndex) const -> CardDB::CardPtr;
    [[nodiscard]] auto getActiveVocables(size_t cardIndex) -> std::set<VocableId>;
    [[nodiscard]] auto getVocableIdsInOrder(size_t cardIndex) const -> std::vector<VocableId>;
    [[nodiscard]] auto getRelevantEase(size_t cardIndex) -> std::map<VocableId, Ease>;

    void setEaseVocable(VocableId, const Ease&);
    void triggerVocable(VocableId, CardId);
    void resetCardsContainingVocable(VocableId vocId);
    void saveProgress() const;
    void addVocableChoice(uint vocId, uint vocIdOldChoice, uint vocIdNewChoice);

private:
    std::shared_ptr<zikhron::Config> config;

    std::shared_ptr<const ZH_Dictionary> zhDictionary;

    AnnotationChoiceMap annotationChoices;
    vocId_vocId_map vocableChoices;
    std::shared_ptr<CardDB> cardDB;
    std::map<VocableId, VocableProgress> progressVocables;
    std::map<CardId, CardProgress> progressCards;

    static void saveJsonToFile(const std::filesystem::path& fn, const nlohmann::json& js);
    static auto loadJsonFromFile(const std::filesystem::path& fn) -> nlohmann::json;
    static auto loadAnnotationChoices(const std::filesystem::path& annotationChoicesPath) -> AnnotationChoiceMap;
    static auto loadVocableChoices(const std::filesystem::path& vocableChoicesPath) -> vocId_vocId_map;

    template<class key_type, class mapped_value>
    static auto jsonToMap(const nlohmann::json& jsonMeta) -> std::map<key_type, mapped_value>;
    static auto loadProgressVocables(
            const std::filesystem::path& progressVocablePath) -> std::map<VocableId, VocableProgress>;
    static auto loadProgressCards(
            const std::filesystem::path& progressCardsPath) -> std::map<CardId, CardProgress>;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    void fillIndexMaps();
    void insertVocabularyOfCard(const CardDB::CardPtr& card);
    static auto getVocableIdsInOrder(const CardDB::CardPtr& card,
                                     const std::map<unsigned, unsigned>& vocableChoices) -> std::vector<VocableId>;
    [[nodiscard]] auto generateVocableIdProgressMap() const -> std::map<VocableId, VocableProgress>;

    std::shared_ptr<utl::index_map<VocableId, VocableMeta>> vocables;
    std::shared_ptr<utl::index_map<CardId, CardMeta>> cards;
};
} // namespace sr
