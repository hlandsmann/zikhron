#pragma once
#include "CardProgress.h"
#include "DataBase.h"
#include "VocableProgress.h"

#include <folly/sorted_vector_types.h>
#include <misc/Config.h>
#include <utils/index_map.h>

struct VocableMeta
{
    VocableProgress progress;
    folly::sorted_vector_set<std::size_t> cardIndices;
};

struct CardMeta
{
    CardProgress progress;
    folly::sorted_vector_set<std::size_t> cardIndices;
};

class WalkableData
{
public:
    WalkableData(std::shared_ptr<zikhron::Config> config);

private:
    DataBase db;
    utl::index_map<VocableMeta> vocables;
    utl::index_map<CardMeta> cards;

    void fillIndexMaps();
    void insertVocabularyOfCard(const CardDB::CardPtr& card);
};
