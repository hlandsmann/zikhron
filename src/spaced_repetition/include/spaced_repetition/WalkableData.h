#pragma once
#include "CardProgress.h"
#include "DataBase.h"
#include "VocableProgress.h"

#include <annotation/ZH_Annotator.h>
#include <folly/sorted_vector_types.h>
#include <misc/Config.h>
#include <utils/index_map.h>

struct VocableMeta
{
    VocableMeta(VocableProgress _progress,
                folly::sorted_vector_set<std::size_t> _cardIndices,
                ZH_Annotator::ZH_dicItemVec dicItemVec);
    VocableProgress progress;
    folly::sorted_vector_set<std::size_t> cardIndices;
    ZH_Annotator::ZH_dicItemVec dicItemVec;
};

struct CardMeta
{
    CardMeta(CardProgress progress, folly::sorted_vector_set<std::size_t> cardIndices);
    CardProgress progress;
    folly::sorted_vector_set<std::size_t> vocableIndices;
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

    static auto getVocableIdsInOrder(const CardDB::CardPtr& card,
                                     const std::map<unsigned, unsigned>& vocableChoices) -> std::vector<uint>;
};
