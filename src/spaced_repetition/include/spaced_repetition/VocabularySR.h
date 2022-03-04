#pragma once

#include <annotation/Ease.h>
#include <annotation/ZH_Annotator.h>
#include <dictionary/ZH_Dictionary.h>
#include <utils/StringU8.h>
#include <iosfwd>
#include <thread>

#ifdef spaced_repetition_internal_include
#include "SR_DataBase.h"
#else
#include <spaced_repetition/SR_DataBase.h>
#endif

class CardDB;
class Card;

class VocabularySR_X {
public:
    VocabularySR_X(const std::map<uint, VocableSR>&,
                   const std::map<uint, CardMeta>&,
                   const std::map<uint, VocableMeta>&);

private:
    void initDataStructure();
    std::jthread worker;

    using id_vocSR_t = std::pair<uint, VocableSR>;
    std::vector<id_vocSR_t> repeatTodayVoc;

    const std::map<uint, VocableSR>& id_vocableSR;
    const std::map<uint, CardMeta>& id_cardMeta;
    const std::map<uint, VocableMeta>& id_vocableMeta;
};

// class VocabularySR_TreeWalker {
// public:
//     VocabularySR_TreeWalker(const std::map<uint, VocableSR>&,
//                             const std::map<uint, CardMeta>&,
//                             const std::map<uint, VocableMeta>&);

// private:
//     struct Group {
//         std::map<uint, VocableMeta> id_vocMeta{};
//         std::map<uint, CardMeta> id_cardMeta{};
//     };
//     auto SplitGroup(const Group& group) -> std::vector<Group>;
//     void ProcessGroup(Group& group);
//     auto OtherCardsWithVocables(const std::map<uint, CardMeta>& id_cm, uint cardId) -> std::set<uint>;

//     using IdCardMeta_vec = std::vector<std::pair<uint, CardMeta>>;
//     auto CardsBestSize(const std::map<uint, CardMeta>& id_cm) -> IdCardMeta_vec;
//     auto RefinedCards(const IdCardMeta_vec&) -> IdCardMeta_vec;

//     std::jthread worker;

//     std::map<uint, VocableSR> id_vocableSR;
//     std::map<uint, CardMeta> id_cardMeta;
//     std::map<uint, VocableMeta> id_vocableMeta;
// };

class VocabularySR {
    using ZH_dicItemVec = std::vector<ZH_Dictionary::Item>;
    static constexpr std::string_view s_content = "content";
    static constexpr std::string_view s_fn_metaVocableSR = "metaVocableSR.json";
    static constexpr std::string_view s_fn_metaCardSR = "metaCardSR.json";
    static constexpr std::string_view s_fn_annotationChoices = "annotationChoices.json";
    static constexpr std::string_view s_path_meta = "/home/harmen/zikhron";

public:
    VocabularySR(CardDB&&, std::shared_ptr<ZH_Dictionary>);
    ~VocabularySR();

    using Item_Id_vt = std::vector<std::pair<ZH_Dictionary::Item, uint>>;
    using Id_Ease_vt = std::map<uint, Ease>;
    using CardInformation = std::tuple<std::unique_ptr<Card>, Item_Id_vt, Id_Ease_vt>;
    auto getCard() -> CardInformation;
    auto addAnnotation(const std::vector<int>& combination, const std::vector<utl::CharU8>& characters)
        -> CardInformation;
    void setEaseLastCard(const Id_Ease_vt&);

private:
    void GenerateFromCards();
    auto CountTotalNewVocablesInSet() -> size_t;
    auto CalculateCardValueSingle(const CardMeta& cm, const std::set<uint>& good) const -> float;
    auto CalculateCardValueSingleNewVoc(const CardMeta& cm, const std::set<uint>& neutral) const
        -> float;
    void InsertVocabulary(const std::set<ZH_Annotator::Item>& cardVocabulary, uint cardId);
    void EraseVocabulary(uint cardId);
    static void SaveJsonToFile(const std::string_view& fn, const nlohmann::json& js);
    void SaveProgress() const;
    void SaveAnnotationChoices() const;
    static auto LoadJsonFromFile(const std::string_view& fn) -> nlohmann::json;
    void LoadProgress();
    void LoadAnnotationChoices();
    void GenerateToRepeatWorkload();
    void CleanUpVocables();

    // Get vocables that would need to be learned with this current cardId
    auto GetActiveVocables_dicEntry(uint cardId) const -> Item_Id_vt;
    auto GetActiveVocables(uint cardId) const -> std::set<uint>;
    auto GetRelevantEase(uint cardId) -> Id_Ease_vt;

    // Calculate which Cards to learn next
    auto GetCardRepeatedVoc() -> std::optional<uint>;
    auto GetCardNewVocStart() -> std::optional<uint>;
    auto GetCardNewVoc() -> std::optional<uint>;

    std::shared_ptr<CardDB> cardDB;
    std::shared_ptr<ZH_Dictionary> zh_dictionary;
    std::map<ZH_dicItemVec, uint> zhdic_vocableMeta;
    std::map<uint, VocableMeta> id_vocableMeta;
    // vocableId -> vocable (aka. ZH_dicItemVec)
    std::map<uint, ZH_dicItemVec> id_vocable;
    /* cardMeta sorted by value */
    std::map<uint, CardMeta> id_cardMeta;
    std::map<uint, CardSR> id_cardSR;
    std::map<uint, VocableSR> id_vocableSR;

    /* ids for to be repeated vocables */
    std::set<uint> ids_repeatTodayVoc;
    /* ids for vocables the student failed */
    std::set<uint> ids_againVoc;
    /* ids for vocables that are to be repeated NOW! - they get moved out of againVoc */
    std::set<uint> ids_nowVoc;

    size_t countOfNewVocablesToday = 0;
    uint activeCardId{};
    bool getCardNeedsCleanup = false;

    using CharacterSequence = std::vector<utl::CharU8>;
    using Combination = std::vector<int>;
    std::map<CharacterSequence, Combination> annotationChoices;
    // std::unique_ptr<VocabularySR_TreeWalker> treeWalker;
    std::unique_ptr<VocabularySR_X> X;
};
