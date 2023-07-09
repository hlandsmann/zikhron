#pragma once

#include <annotation/TextCard.h>
#ifdef spaced_repetition_internal_include
#include "DataBase.h"
#else
#include <spaced_repetition/DataBase.h>
#endif

class TreeWalker {
public:
    TreeWalker(const std::shared_ptr<CardDB>&, const std::shared_ptr<ZH_Dictionary>&);
};
