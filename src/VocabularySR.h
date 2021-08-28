#pragma once

#include <Ease.h>
#include <ZH_Annotator.h>
#include <ZH_Dictionary.h>
#include <utils/StringU8.h>
#include <cppcoro/generator.hpp>
#include <iosfwd>
#include <nlohmann/json_fwd.hpp>
#include <thread>

class CardDB;
class Card;

struct VocableMeta {
    // uint id = 0;
    std::set<uint> cardIds;
};

struct CardMeta {
    float value = 0;
    std::set<uint> vocableIds;

    uint cardId = 0;
};

struct VocableSR {
    static constexpr int pause_time_minutes = 5;
    using pair_t = std::pair<uint, VocableSR>;
    static constexpr std::string_view s_id = "id";
    static constexpr std::string_view s_ease_factor = "ease_factor";
    static constexpr std::string_view s_interval_day = "interval_day";
    static constexpr std::string_view s_last_seen = "last_seen";
    static constexpr std::string_view s_indirect_view = "indirect_view";
    static constexpr std::string_view s_indirect_interval_day = "indirect_interval_day";

    float easeFactor = 0.f;
    float intervalDay = 0.f;
    std::time_t lastSeen{};
    std::time_t indirectView{};
    uint indirectIntervalDay = 0;

    void advanceByEase(Ease);
    bool advanceIndirectly();
    auto urgency() const -> float;
    auto pauseTimeOver() const -> bool;
    auto isToBeRepeatedToday() const -> bool;
    auto isAgainVocable() const -> bool;
    static auto toJson(const pair_t&) -> nlohmann::json;
    static auto fromJson(const nlohmann::json&) -> pair_t;
};

struct CardSR {
    using pair_t = std::pair<uint, CardSR>;
    static constexpr std::string_view s_id = "id";
    static constexpr std::string_view s_last_seen = "last_seen";
    static constexpr std::string_view s_view_count = "view_count";

    std::time_t lastSeen{};
    uint viewCount{};

    void ViewNow();
    static auto toJson(const pair_t&) -> nlohmann::json;
    static auto fromJson(const nlohmann::json&) -> pair_t;
};

class VocabluarySR_TreeWalker {
public:
    VocabluarySR_TreeWalker(const std::map<uint, VocableSR>&,
                            const std::map<uint, CardMeta>&,
                            const std::map<uint, VocableMeta>&);
    ~VocabluarySR_TreeWalker() = default;

private:
    struct Group {
        std::map<uint, VocableMeta> id_vocMeta{};
        std::map<uint, CardMeta> id_cardMeta{};
    };
    auto SplitGroup(const Group& group) -> std::vector<Group>;
    void ProcessGroup(Group& group);
    auto OtherCardsWithVocables(const std::map<uint, CardMeta>& id_cm, uint cardId)
        -> std::set<uint>;

    using IdCardMeta_vec = std::vector<std::pair<uint, CardMeta>>;
    auto CardsBestSize(const std::map<uint, CardMeta>& id_cm) -> IdCardMeta_vec;
    auto RefinedCards(const IdCardMeta_vec&) -> IdCardMeta_vec;

    std::jthread worker;

    std::map<uint, VocableSR> id_vocableSR;
    std::map<uint, CardMeta> id_cardMeta;
    std::map<uint, VocableMeta> id_vocableMeta;
};

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
    auto addAnnotation(const std::vector<int>& combination, const std::vector<utl::ItemU8>& characters)
        -> CardInformation;
    void setEaseLastCard(const Id_Ease_vt&);

private:
    void GenerateFromCards();
    auto CalculateCardValueSingle(const CardMeta& cm, const std::set<uint>& good) const -> float;
    auto CalculateCardValueSingleNewVoc(const CardMeta& cm, const std::set<uint>& neutral) const
        -> float;
    void InsertVocabulary(const std::set<ZH_Annotator::Item>& cardVocabulary, uint cardId);
    void EraseVocabulary(uint cardId);
    static void SaveJsonToFile(const std::string_view& fn, const nlohmann::json& js);
    void SaveProgress();
    void SaveAnnotationChoices();
    static auto LoadJsonFromFile(const std::string_view& fn) -> nlohmann::json;
    void LoadProgress();
    void LoadAnnotationChoices();
    void GenerateToRepeatWorkload();
    void CleanUpVocables();

    // Get vocables that would need to be learned with this current cardId
    auto GetRelevantVocables(uint cardId) -> Item_Id_vt;
    auto GetRelevantEase(uint cardId) -> Id_Ease_vt;
    auto GetActiveVocables(uint cardId) -> std::set<uint>;

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

    uint countOfNewVocablesToday = 0;
    uint activeCardId{};
    bool getCardNeedsCleanup = false;

    using CharacterSequence = std::vector<utl::ItemU8>;
    using Combination = std::vector<int>;
    std::map<CharacterSequence, Combination> annotationChoices;
    std::unique_ptr<VocabluarySR_TreeWalker> treeWalker;
};

struct counting_iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = int;
    using difference_type = int;
    using pointer = int*;
    using reference = int&;
    size_t count;
    int ignore;
    counting_iterator& operator++() {
        ++count;
        return *this;
    }
    counting_iterator operator++(int) {
        counting_iterator temp = *this;
        ++*this;
        return temp;
    }
    int& operator*() { return ignore; }

    struct black_hole {
        void operator=(uint) {}
    };
};
