#pragma once

#include <ZH_Annotator.h>
#include <ZH_Dictionary.h>
#include <utils/StringU8.h>
#include <map>
#include <memory>
#include <set>

class CardDB;

struct VocableMeta {
    int id;
    std::set<int> cardIds;
};

class VocabularySR {
    using ZH_dicItemVec = std::vector<ZH_Dictionary::Item>;

public:
    VocabularySR(CardDB &&, std::shared_ptr<ZH_Dictionary>);
    ~VocabularySR();

private:
    void GenerateFromCards();
    void InsertVocabulary(const std::set<ZH_Annotator::Item> &cardVocabulary, int cardId);
    auto GetNextFreeId() -> int;
    std::shared_ptr<CardDB> cardDB;
    std::shared_ptr<ZH_Dictionary> zh_dictionary;
    std::map<ZH_dicItemVec, VocableMeta> vocables;
    std::map<int, ZH_dicItemVec> id_vocable;
    std::set<utl::ItemU8> allCharacters;
};
