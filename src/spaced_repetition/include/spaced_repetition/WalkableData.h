#pragma once
#include "CardProgress.h"
#include "DataBase.h"
#include "VocableProgress.h"

#include <annotation/ZH_Annotator.h>
#include <folly/sorted_vector_types.h>
#include <misc/Config.h>
#include <utils/index_map.h>

#include <cstddef>

struct VocableMeta
{
    VocableMeta(VocableProgress _progress,
                folly::sorted_vector_set<std::size_t> _cardIndices,
                ZH_Annotator::ZH_dicItemVec dicItemVec);
    [[nodiscard]] auto Progress() const -> VocableProgress;
    [[nodiscard]] auto CardIndices() const -> folly::sorted_vector_set<std::size_t>;

private:
    VocableProgress progress;
    folly::sorted_vector_set<std::size_t> cardIndices;
    ZH_Annotator::ZH_dicItemVec dicItemVec;
};

struct CardMeta
{
    CardMeta(CardProgress progress, folly::sorted_vector_set<std::size_t> _vocableIndices);
    [[nodiscard]] auto Progress() const -> CardProgress;
    [[nodiscard]] auto VocableIndices() const -> folly::sorted_vector_set<std::size_t>;

private:
    CardProgress progress;
    folly::sorted_vector_set<std::size_t> vocableIndices;
};

class WalkableData
{
public:
    WalkableData(std::shared_ptr<zikhron::Config> config);
    [[nodiscard]] auto Vocables() const -> utl::index_map<VocableMeta>;
    [[nodiscard]] auto Cards() const -> utl::index_map<CardMeta>;

    struct TimingAndValue
    {
        int timing;
        std::size_t nVocables;
    };
    [[nodiscard]] auto timingAndNVocables(
            const CardMeta& card,
            const folly::sorted_vector_set<std::size_t>& deadVocables) const -> TimingAndValue;
    [[nodiscard]] auto timingAndNVocables(const CardMeta& card) const -> TimingAndValue;

private:
    DataBase db;
    utl::index_map<VocableMeta> vocables;
    utl::index_map<CardMeta> cards;

    void fillIndexMaps();
    void insertVocabularyOfCard(const CardDB::CardPtr& card);

    static auto getVocableIdsInOrder(const CardDB::CardPtr& card,
                                     const std::map<unsigned, unsigned>& vocableChoices) -> std::vector<uint>;
};
