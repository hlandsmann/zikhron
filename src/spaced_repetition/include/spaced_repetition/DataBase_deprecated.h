#pragma once

#include <annotation/Ease.h>
#include <annotation/TextCard.h>
#include <annotation/ZH_Annotator.h>
#include <dictionary/ZH_Dictionary.h>
#include <ctime>
#include <nlohmann/json_fwd.hpp>
#include <set>
#include <string_view>
#include "CardProgress.h"
#include "VocableProgress.h"

struct VocableMetaDeprecated {
    // uint id = 0;
    std::set<uint> cardIds;
};

struct CardMetaDeprecated {
    float value = 0;
    std::set<uint> vocableIds;

    uint cardId = 0;
};

class SR_DataBase {
    static constexpr std::string_view s_content = "content";

    static constexpr std::string_view s_path_meta = "/home/harmen/zikhron";
    static constexpr std::string_view s_fn_metaVocableSR = "metaVocableSR.json";
    static constexpr std::string_view s_fn_metaCardSR = "metaCardSR.json";
    static constexpr std::string_view s_fn_annotationChoices = "annotationChoices.json";
    static constexpr std::string_view s_fn_vocableChoices = "vocableChoices.json";

public:
    SR_DataBase(const std::shared_ptr<CardDB>&, const std::shared_ptr<ZH_Dictionary>&);
    SR_DataBase(SR_DataBase&&) = delete;
    SR_DataBase(const SR_DataBase&) = delete;
    auto operator=(SR_DataBase&&) -> SR_DataBase = delete;
    auto operator=(const SR_DataBase&) -> SR_DataBase = delete;
    ~SR_DataBase();

    [[nodiscard]] auto Id_cardSR() const -> const std::map<uint, CardProgress>& { return id_cardSR; };
    [[nodiscard]] auto Id_vocableSR() const -> const std::map<uint, VocableProgress>& { return id_vocableSR; };
    [[nodiscard]] auto Id_cardMeta() const -> const std::map<uint, CardMetaDeprecated>& { return id_cardMeta; };
    [[nodiscard]] auto Id_vocableMeta() const -> const std::map<uint, VocableMetaDeprecated>& {
        return id_vocableMeta;
    };
    [[nodiscard]] auto Ids_repeatTodayVoc() const -> const std::set<uint>& {
        return ids_repeatTodayVoc;
    };
    [[nodiscard]] auto Ids_againVoc() const -> const std::set<uint>& { return ids_againVoc; };
    [[nodiscard]] auto Ids_nowVoc() const -> const std::set<uint>& { return ids_nowVoc; };

    [[nodiscard]] auto GetVocableIdsInOrder(uint cardId) const -> std::vector<uint>;
    void SetEase(uint vocId, Ease ease);
    void ViewCard(uint cardId);
    void AdvanceIndirectlySeenVocables(uint cardId);
    void AdvanceFailedVocables();
    void AddAnnotation(const ZH_Annotator::Combination& combination,
                       const std::vector<utl::CharU8>& characterSequence,
                       uint activeCardId);
    void AddVocableChoice(uint vocId, uint vocIdOldChoice, uint vocIdNewChoice);

private:
    using ZH_dicItemVec = std::vector<ZH_Dictionary::Entry>;

    static void SaveJsonToFile(const std::string_view& fn, const nlohmann::json& js);
    static auto LoadJsonFromFile(const std::string_view& fn) -> nlohmann::json;
    void LoadAnnotationChoices();
    void SaveAnnotationChoices() const;
    void LoadVocableChoices();
    void SaveVocableChoices() const;
    void GenerateFromCards();
    void EraseVocabularyOfCard(uint cardId);
    void InsertVocabularyOfCard(uint cardId, const CardDB::CardPtr& card);
    void LoadProgress();
    void SaveProgress() const;
    void GenerateToRepeatWorkload();
    void CleanUpVocables(std::set<uint> ignoreVocableIds);

    std::shared_ptr<CardDB> cardDB;
    std::shared_ptr<const ZH_Dictionary> zh_dictionary;

    // vocableId -> vocable (aka. ZH_dicItemVec)
    std::map<uint, ZH_dicItemVec> id_vocable;

    std::map<uint, CardProgress> id_cardSR;
    std::map<uint, VocableProgress> id_vocableSR;
    std::map<uint, CardMetaDeprecated> id_cardMeta;
    std::map<uint, VocableMetaDeprecated> id_vocableMeta;

    /* ids for to be repeated vocables */
    std::set<uint> ids_repeatTodayVoc;
    /* ids for vocables the student failed */
    std::set<uint> ids_againVoc;
    /* ids for vocables that are to be repeated NOW! - they get moved out of againVoc */
    std::set<uint> ids_nowVoc;

    std::map<uint, uint> id_id_vocableChoices;

    using CharacterSequence = std::vector<utl::CharU8>;
    using Combination = std::vector<int>;
    std::map<CharacterSequence, Combination> annotationChoices;

    std::map<std::string, uint> zhdic_vocableMeta;
};
