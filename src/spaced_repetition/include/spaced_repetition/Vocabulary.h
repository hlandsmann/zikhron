#pragma once
#include <misc/Identifier.h>
#include "VocableProgress.h"

#include <annotation/Card.h>
#include <annotation/Ease.h>
#include <annotation/ZH_Annotator.h>
#include <dictionary/ZH_Dictionary.h>
#include <utils/StringU8.h>

#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <sys/types.h>

#ifdef spaced_repetition_internal_include
#include "DataBase_deprecated.h"
#else
#include <spaced_repetition/DataBase_deprecated.h>
#endif

class CardDB;
struct CardProgress;

class VocabularySR
{
    using ZH_dicItemVec = std::vector<ZH_Dictionary::Entry>;
    static constexpr std::string_view s_fn_metaVocableSR = "metaVocableSR.json";
    static constexpr std::string_view s_fn_metaCardSR = "metaCardSR.json";
    static constexpr std::string_view s_fn_annotationChoices = "annotationChoices.json";
    static constexpr std::string_view s_fn_vocableChoices = "vocableChoices.json";
    static constexpr std::string_view s_path_meta = "/home/harmen/zikhron";

public:
    VocabularySR(const std::shared_ptr<CardDB>&, const std::shared_ptr<ZH_Dictionary>&);

    using VocableIds_vt = std::vector<VocableId>;
    using Id_Ease_vt = std::map<VocableId, Ease>;
    using CardInformation = std::tuple<std::unique_ptr<Card>, VocableIds_vt, Id_Ease_vt>;
    auto getNextCardChoice(std::optional<uint> preferedCardId = {}) -> CardInformation;
    [[nodiscard]] auto getCardFromId(uint id) const -> std::optional<CardInformation>;
    auto AddAnnotation(const ZH_Annotator::Combination& combination,
                       const std::vector<utl::CharU8>& characters) -> CardInformation;
    auto AddVocableChoice(uint vocId, uint vocIdOldChoice, uint vocIdNewChoice) -> CardInformation;
    void setEaseLastCard(const Id_Ease_vt&);

private:
    auto CountTotalNewVocablesInSet() -> size_t;
    [[nodiscard]] auto CalculateCardValueSingle(const CardMetaDeprecated& cm, const std::set<uint>& good) const
            -> float;
    [[nodiscard]] auto CalculateCardValueSingleNewVoc(const CardMetaDeprecated& cm,
                                                      const std::set<uint>& neutral) const -> float;

    // Get vocables that would need to be learned with this current cardId
    // auto GetActiveVocables_dicEntry(uint cardId) const -> Item_Id_vt;
    [[nodiscard]] auto GetActiveVocables(uint cardId) const -> std::set<uint>;
    [[nodiscard]] auto GetRelevantEase(uint cardId) const -> Id_Ease_vt;

    // Calculate which Cards to learn next
    auto GetCardRepeatedVoc() -> std::optional<uint>;
    auto GetCardNewVocStart() -> std::optional<uint>;
    auto GetCardNewVoc() -> std::optional<uint>;

    std::shared_ptr<CardDB> cardDB;
    std::shared_ptr<ZH_Dictionary> zh_dictionary;
    std::map<std::string, uint> zhdic_vocableMeta;

    SR_DataBase sr_db;

    const std::map<uint, CardProgress>& id_cardSR = sr_db.Id_cardSR();

    const std::map<uint, VocableProgress>& id_vocableSR = sr_db.Id_vocableSR();
    const std::map<uint, CardMetaDeprecated>& id_cardMeta = sr_db.Id_cardMeta();
    const std::map<uint, VocableMetaDeprecated>& id_vocableMeta = sr_db.Id_vocableMeta();

    /* ids for to be repeated vocables */
    const std::set<uint>& ids_repeatTodayVoc = sr_db.Ids_repeatTodayVoc();
    /* ids for vocables the student failed */
    const std::set<uint>& ids_againVoc = sr_db.Ids_againVoc();
    /* ids for vocables that are to be repeated NOW! - they get moved out of againVoc */
    const std::set<uint>& ids_nowVoc = sr_db.Ids_nowVoc();

    size_t countOfNewVocablesToday = 0;
    std::optional<uint> activeCardId{};
    bool getCardNeedsCleanup = false;
};
