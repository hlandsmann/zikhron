#include "VocabularySR.h"
#include <ZH_Annotator.h>
#include <algorithm>

VocabularySR::VocabularySR(CardDB &&_cardDB, std::shared_ptr<ZH_Dictionary> _zh_dictionary)
    : cardDB(std::make_shared<CardDB>(std::move(_cardDB))), zh_dictionary(_zh_dictionary) {}
