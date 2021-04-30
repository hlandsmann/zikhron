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
    uint id = 0;
    std::set<uint> cardIds;
};

struct CardMeta {
    float value = 0;
    std::set<uint> vocableIds;

    uint cardId = 0;
};

struct VocableSR {
    using day_t = std::chrono::duration<double, std::ratio<60*60*24>>;
    void advanceByEase(Ease);
    std::chrono::time_point<std::chrono::steady_clock> last_seen{};

    float ease_factor = 0;
    day_t interval_day{};
};

struct CardSR {
    std::chrono::time_point<std::chrono::system_clock> last_seen{};
};

class VocabularySR {
    using ZH_dicItemVec = std::vector<ZH_Dictionary::Item>;

public:
    VocabularySR(CardDB &&, std::shared_ptr<ZH_Dictionary>);
    ~VocabularySR();

    auto getCard() -> std::pair<std::unique_ptr<Card>, std::vector<ZH_Dictionary::Item>>;
    void setEaseLastCard(Ease ease);

private:
    void GenerateFromCards();
    void CalculateCardValue();
    void InsertVocabulary(const std::set<ZH_Annotator::Item> &cardVocabulary, uint cardId);
    auto GetNextFreeId() -> uint;
    void SaveProgress();

    // Get vocables that would need to be learned with this current cardId
    auto GetRelevantVocables(uint cardId) -> std::vector<ZH_Dictionary::Item>;

    std::shared_ptr<CardDB> cardDB;
    std::shared_ptr<ZH_Dictionary> zh_dictionary;
    std::map<ZH_dicItemVec, VocableMeta> vocables;

    // vocableId -> vocable (aka. ZH_dicItemVec)
    std::map<uint, ZH_dicItemVec> id_vocable;
    std::set<utl::ItemU8> allCharacters;
    std::vector<CardMeta> cardMeta;
    std::map<uint, CardMeta> id_cardMeta;
    std::vector<CardSR> cardSR;
    std::map<uint, VocableSR> id_vocableSR;

    std::set<uint> ids_activeVocables;
};
