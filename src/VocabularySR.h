#pragma once

#include <Ease.h>
#include <ZH_Annotator.h>
#include <ZH_Dictionary.h>
#include <utils/StringU8.h>
#include <iosfwd>
#include <nlohmann/json_fwd.hpp>

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
    using pair_t = std::pair<uint, VocableSR>;
    static constexpr std::string_view s_id = "id";
    static constexpr std::string_view s_ease_factor = "ease_factor";
    static constexpr std::string_view s_interval_day = "interval_day";
    static constexpr std::string_view s_last_seen = "last_seen";

    float easeFactor = 0.f;
    float intervalDay = 0.f;
    std::time_t lastSeen{};

    void advanceByEase(Ease);
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

class VocabularySR {
    using ZH_dicItemVec = std::vector<ZH_Dictionary::Item>;
    static constexpr std::string_view s_content = "content";
    static constexpr std::string_view s_fn_metaVocableSR = "metaVocableSR.json";
    static constexpr std::string_view s_fn_metaCardSR = "metaCardSR.json";
    static constexpr std::string_view s_path_meta = "/home/harmen/zikhron";

public:
    VocabularySR(CardDB&&, std::shared_ptr<ZH_Dictionary>);
    ~VocabularySR();

    using Item_Id_vt = std::vector<std::pair<ZH_Dictionary::Item, uint>>;
    using Id_Ease_vt = std::map<uint, Ease>;
    auto getCard() -> std::tuple<std::unique_ptr<Card>, Item_Id_vt, Id_Ease_vt>;
    void setEaseLastCard(const Id_Ease_vt&);

private:
    void GenerateFromCards();
    auto CalculateCardValueSingle(const CardMeta& cm, const std::set<uint>& good) const -> float;
    auto CalculateCardValueSingleNewVoc(const CardMeta& cm, const std::set<uint>& neutral) const
        -> float;
    void InsertVocabulary(const std::set<ZH_Annotator::Item>& cardVocabulary, uint cardId);
    auto GetNextFreeId() -> uint;
    void SaveProgress();
    void LoadProgress();
    void GenerateToRepeatWorkload();

    // Get vocables that would need to be learned with this current cardId
    auto GetRelevantVocables(uint cardId) -> Item_Id_vt;
    auto GetRelevantEase(uint cardId) -> Id_Ease_vt;
    auto GetActiveVocables(uint cardId) -> std::set<uint>;

    // Calculate which Cards to learn next
    auto GetCardRepeatedVoc() -> std::optional<uint>;
    auto GetCardNewVoc() -> std::optional<uint>;

    std::shared_ptr<CardDB> cardDB;
    std::shared_ptr<ZH_Dictionary> zh_dictionary;
    std::map<ZH_dicItemVec, uint> zhdic_vocableMeta;
    std::map<uint, VocableMeta> id_vocableMeta;
    // vocableId -> vocable (aka. ZH_dicItemVec)
    std::map<uint, ZH_dicItemVec> id_vocable;
    std::set<utl::ItemU8> allCharacters;
    /* cardMeta sorted by value */
    std::map<uint, CardMeta> id_cardMeta;
    std::map<uint, CardSR> id_cardSR;
    std::map<uint, VocableSR> id_vocableSR;

    /* ids of active Vocables for current Card */
    std::set<uint> ids_activeVoc;

    /* ids for to be repeated vocables */
    std::set<uint> ids_repeatTodayVoc;
    /* ids for vocables the student failed */
    std::set<uint> ids_againVoc;
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
    // black_hole operator*() { return black_hole(); }
};
