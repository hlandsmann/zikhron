#pragma once

#include "DataBase.h"
#include "VocableProgress.h"

#include <misc/Config.h>
#include <utils/index_map.h>

struct VocableMeta
{
};

struct CardMeta
{
};

class WalkableData
{
public:
    WalkableData(std::shared_ptr<zikhron::Config> config);

private:
    DataBase db;
    utl::index_map<VocableProgress> vocables;
    utl::index_map<CardMeta> cards;
};
