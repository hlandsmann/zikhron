#pragma once

#include <Ease.h>
#include <ZH_Annotator.h>
#include <ZH_Dictionary.h>
#include <utils/StringU8.h>
#include <chrono>
#include <map>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <set>
#include <tuple>

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
    static constexpr std::string_view s_id = "id";
    static constexpr std::string_view s_ease_factor = "ease_factor";
    static constexpr std::string_view s_interval_day = "interval_day";
    static constexpr std::string_view s_last_seen = "last_seen";

    uint vocId = 0;
    float ease_factor = 0.f;
    float interval_day = 0.f;
    std::time_t last_seen{};

    void advanceByEase(Ease);
    auto toJson() const -> nlohmann::json;
    static auto fromJson(const nlohmann::json&) -> VocableSR;
};

struct CardSR {
    static constexpr std::string_view s_id = "id";
    static constexpr std::string_view s_last_seen = "last_seen";

    uint cardId = 0;
    std::time_t last_seen{};

    auto toJson() const -> nlohmann::json;
    static auto fromJson(const nlohmann::json&) -> CardSR;
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

    auto getCard() -> std::pair<std::unique_ptr<Card>, std::vector<ZH_Dictionary::Item>>;
    void setEaseLastCard(Ease ease);

private:
    void GenerateFromCards();
    void CalculateCardValues();
    auto CalculateCardValueSingle(const CardMeta& cm, const std::set<uint>& good) const -> float;
    auto CalculateCardValueSingleNewVoc(const CardMeta& cm, const std::set<uint>& neutral) const
        -> float;
    void InsertVocabulary(const std::set<ZH_Annotator::Item>& cardVocabulary, uint cardId);
    auto GetNextFreeId() -> uint;
    void SaveProgress();
    void LoadProgress();
    void GenerateToRepeatWorkload();

    // Get vocables that would need to be learned with this current cardId
    auto GetRelevantVocables(uint cardId) -> std::vector<ZH_Dictionary::Item>;

    std::shared_ptr<CardDB> cardDB;
    std::shared_ptr<ZH_Dictionary> zh_dictionary;
    std::map<ZH_dicItemVec, uint> zhdic_vocableMeta;
    std::map<uint, VocableMeta> id_vocableMeta;
    // vocableId -> vocable (aka. ZH_dicItemVec)
    std::map<uint, ZH_dicItemVec> id_vocable;
    std::set<utl::ItemU8> allCharacters;
    /* cardMeta sorted by value */
    std::vector<std::shared_ptr<CardMeta>> cardMeta;
    std::map<uint, std::shared_ptr<CardMeta>> id_cardMeta;
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
    size_t count;
    counting_iterator& operator++() {
        ++count;
        return *this;
    }

    struct black_hole {
        void operator=(uint) {}
    };
    black_hole operator*() { return black_hole(); }
};
