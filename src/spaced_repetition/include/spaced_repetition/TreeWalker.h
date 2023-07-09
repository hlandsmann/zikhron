#pragma once

#include <annotation/TextCard.h>
#include "DataBase.h"

#include <memory>

class TreeWalker {
public:
    TreeWalker(const std::shared_ptr<CardDB>&, const std::shared_ptr<ZH_Dictionary>&);

private:
    std::shared_ptr<CardDB> cardDB;
    std::shared_ptr<ZH_Dictionary> zh_dictionary;
    SR_DataBase sr_db;
};
