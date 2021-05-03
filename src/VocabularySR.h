#pragma once

#include <Ease.h>
#include <ZH_Annotator.h>
#include <ZH_Dictionary.h>
#include <utils/StringU8.h>
#include <chrono>
#include <map>
#include <memory>
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
    void advanceByEase(Ease);
    static constexpr std::string_view s_id = "id";
    static constexpr std::string_view s_ease_factor = "ease_factor";
    static constexpr std::string_view s_interval_day = "interval_day";
    static constexpr std::string_view s_last_seen = "last_seen";

    float ease_factor = 0.f;
    float interval_day = 0.f;
    std::time_t last_seen{};
};

struct CardSR {
    std::chrono::time_point<std::chrono::system_clock> last_seen{};
};

class VocabularySR {
    using ZH_dicItemVec = std::vector<ZH_Dictionary::Item>;
    static constexpr std::string_view s_content = "content";
    static constexpr std::string_view s_fn_metaVocableSR = "metaVocableSR.json";
    static constexpr std::string_view s_path_meta = "/var/tmp/portage/zikhron";
public:
    VocabularySR(CardDB &&, std::shared_ptr<ZH_Dictionary>);
    ~VocabularySR();

    auto getCard() -> std::pair<std::unique_ptr<Card>, std::vector<ZH_Dictionary::Item>>;
    void setEaseLastCard(Ease ease);

private:
    void GenerateFromCards();
    void CalculateCardValues();
    auto CalculateCardValueSingle(uint cardId) -> float;
    void InsertVocabulary(const std::set<ZH_Annotator::Item> &cardVocabulary, uint cardId);
    auto GetNextFreeId() -> uint;
    void SaveProgress();
    void LoadProgress();

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
    std::vector<CardSR> cardSR;
    std::map<uint, VocableSR> id_vocableSR;

    std::set<uint> ids_activeVoc;
    std::set<uint> ids_repeatTodayVoc;
    std::set<uint> ids_againVoc;
};
