#pragma once

#include <TextCard.h>
#include <ZH_Dictionary.h>
#include <memory>

struct Vocable {
    using ZH_dicItemVec = std::vector<ZH_Dictionary::Item>;
    int id;
    ZH_dicItemVec dicItemVec;

};

class VocabularySR {
public:
    VocabularySR(CardDB &&, std::shared_ptr<ZH_Dictionary>);

private:
    std::shared_ptr<CardDB> cardDB;
    std::shared_ptr<ZH_Dictionary> zh_dictionary;
};
