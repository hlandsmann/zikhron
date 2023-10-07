#pragma once
#include <misc/Identifier.h>
#include <annotation/Card.h>
#include <annotation/Ease.h>
#include <annotation/ZH_Annotator.h>
#include <dictionary/ZH_Dictionary.h>
#include <utils/StringU8.h>

#include <map>
#include <memory>
#include <string>
#include <vector>
// #include <ctime>
#include "CardProgress.h"
#include "VocableProgress.h"

#include <nlohmann/json_fwd.hpp>
#include <set>
#include <string_view>

struct VocableMetaDeprecated
{
    // uint id = 0;
    std::set<unsigned> cardIds;
};

struct CardMetaDeprecated
{
    float value = 0;
    std::set<unsigned> vocableIds;

    unsigned cardId = 0;
};

class SR_DataBase
{
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

    [[nodiscard]] auto Id_cardSR() const -> const std::map<unsigned, CardProgress>& { return id_cardSR; };
    [[nodiscard]] auto Id_vocableSR() const -> const std::map<unsigned, VocableProgress>& { return id_vocableSR; };
    [[nodiscard]] auto Id_cardMeta() const -> const std::map<unsigned, CardMetaDeprecated>& { return id_cardMeta; };
    [[nodiscard]] auto Id_vocableMeta() const -> const std::map<unsigned, VocableMetaDeprecated>&
    {
        return id_vocableMeta;
    };
    [[nodiscard]] auto Ids_repeatTodayVoc() const -> const std::set<unsigned>&
    {
        return ids_repeatTodayVoc;
    };
    [[nodiscard]] auto Ids_againVoc() const -> const std::set<unsigned>& { return ids_againVoc; };
    [[nodiscard]] auto Ids_nowVoc() const -> const std::set<unsigned>& { return ids_nowVoc; };

    [[nodiscard]] auto GetVocableIdsInOrder(unsigned cardId) const -> std::vector<VocableId>;
    void SetEase(unsigned vocId, Ease ease);
    void ViewCard(unsigned cardId);
    void AdvanceIndirectlySeenVocables(unsigned cardId);
    void AdvanceFailedVocables();
    void AddAnnotation(const ZH_Annotator::Combination& combination,
                       const std::vector<utl::CharU8>& characterSequence,
                       unsigned activeCardId);
    void AddVocableChoice(unsigned vocId, unsigned vocIdOldChoice, unsigned vocIdNewChoice);

private:
    using ZH_dicItemVec = std::vector<ZH_Dictionary::Entry>;

    static void SaveJsonToFile(const std::string_view& fn, const nlohmann::json& js);
    static auto LoadJsonFromFile(const std::string_view& fn) -> nlohmann::json;
    void LoadAnnotationChoices();
    void SaveAnnotationChoices() const;
    void LoadVocableChoices();
    void SaveVocableChoices() const;
    void GenerateFromCards();
    void EraseVocabularyOfCard(unsigned cardId);
    void InsertVocabularyOfCard(unsigned cardId, const CardDB::CardPtr& card);
    void LoadProgress();
    void SaveProgress() const;
    void GenerateToRepeatWorkload();
    void CleanUpVocables(std::set<unsigned> ignoreVocableIds);

    std::shared_ptr<CardDB> cardDB;
    std::shared_ptr<const ZH_Dictionary> zh_dictionary;

    // vocableId -> vocable (aka. ZH_dicItemVec)
    std::map<unsigned, ZH_dicItemVec> id_vocable;

    std::map<unsigned, CardProgress> id_cardSR;
    std::map<unsigned, VocableProgress> id_vocableSR;
    std::map<unsigned, CardMetaDeprecated> id_cardMeta;
    std::map<unsigned, VocableMetaDeprecated> id_vocableMeta;

    /* ids for to be repeated vocables */
    std::set<unsigned> ids_repeatTodayVoc;
    /* ids for vocables the student failed */
    std::set<unsigned> ids_againVoc;
    /* ids for vocables that are to be repeated NOW! - they get moved out of againVoc */
    std::set<unsigned> ids_nowVoc;

    std::map<unsigned, unsigned> id_id_vocableChoices;

    using CharacterSequence = std::vector<utl::CharU8>;
    using Combination = std::vector<int>;
    std::map<CharacterSequence, Combination> annotationChoices;

    std::map<std::string, unsigned> zhdic_vocableMeta;
};
