#pragma once
#include "CardProgress.h"
#include "DataBase.h"
#include "VocableProgress.h"

#include <annotation/Card.h>
#include <annotation/Ease.h>
#include <annotation/ZH_Annotator.h>
#include <folly/sorted_vector_types.h>
#include <misc/Config.h>
#include <utils/index_map.h>

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include <sys/types.h>
namespace sr {
using index_set = folly::sorted_vector_set<std::size_t>;

struct VocableMeta
{
    VocableMeta(VocableProgress _progress,
                folly::sorted_vector_set<std::size_t> _cardIndices,
                ZH_Annotator::ZH_dicItemVec dicItemVec);
    [[nodiscard]] auto Progress() const -> VocableProgress;
    [[nodiscard]] auto CardIndices() const -> const folly::sorted_vector_set<std::size_t>&;
    void cardIndices_insert(std::size_t cardIndex);

private:
    VocableProgress progress;
    folly::sorted_vector_set<std::size_t> cardIndices;
    ZH_Annotator::ZH_dicItemVec dicItemVec;
};

struct CardMeta
{
    CardMeta(CardProgress progress, folly::sorted_vector_set<std::size_t> _vocableIndices);
    [[nodiscard]] auto Progress() const -> CardProgress;
    [[nodiscard]] auto VocableIndices() const -> const folly::sorted_vector_set<std::size_t>&;
    void vocableIndices_insert(std::size_t vocableIndex);

private:
    CardProgress progress;
    folly::sorted_vector_set<std::size_t> vocableIndices;
};

class WalkableData
{
public:
    WalkableData(std::shared_ptr<zikhron::Config> config);
    [[nodiscard]] auto Vocables() const -> const utl::index_map<VocableMeta>&;
    [[nodiscard]] auto Cards() const -> const utl::index_map<CardMeta>&;

    struct TimingAndVocables
    {
        int timing{};
        index_set vocables{};
    };
    [[nodiscard]] auto timingAndNVocables(
            const CardMeta& card,
            const folly::sorted_vector_set<std::size_t>& deadVocables) const -> TimingAndVocables;
    [[nodiscard]] auto timingAndNVocables(const CardMeta& card) const -> TimingAndVocables;
    [[nodiscard]] auto timingAndNVocables(size_t cardIndex) const -> TimingAndVocables;
    [[nodiscard]] auto getActiveVocables(size_t cardIndex) const -> std::set<uint>;

    [[nodiscard]] auto getVocableIdsInOrder(uint cardId) const -> std::vector<uint>;
    [[nodiscard]] auto getRelevantEase(uint cardId) const -> std::map<uint, Ease>;

private:
    DataBase db;
    utl::index_map<VocableMeta> vocables;
    utl::index_map<CardMeta> cards;

    std::function<VocableProgress(std::size_t)>
            vocable_progress = [this](std::size_t vocableIndex) { return vocables[vocableIndex].Progress(); };
    std::function<folly::sorted_vector_set<std::size_t>(std::size_t)>
            vocable_cardIndices = [this](std::size_t vocableIndex) { return vocables[vocableIndex].CardIndices(); };
    std::function<CardProgress(std::size_t)>
            card_progress = [this](std::size_t cardIndex) { return cards[cardIndex].Progress(); };
    std::function<folly::sorted_vector_set<std::size_t>(std::size_t)>
            card_vocableIndices = [this](std::size_t cardIndex) { return cards[cardIndex].VocableIndices(); };

    std::function<std::pair<std::size_t, VocableProgress>(std::size_t)>
            index_vocableProgress = [this](std::size_t vocableIndex)
            -> std::pair<std::size_t, VocableProgress> {
        return {vocableIndex, vocables[vocableIndex].Progress()};
    };
    std::function<std::pair<std::size_t, folly::sorted_vector_set<std::size_t>>(std::size_t)>
            index_vocableCardIndices = [this](std::size_t vocableIndex)
            -> std::pair<std::size_t, folly::sorted_vector_set<std::size_t>> {
        return {vocableIndex, vocables[vocableIndex].CardIndices()};
    };
    std::function<std::pair<std::size_t, CardProgress>(std::size_t)>
            index_cardProgress = [this](std::size_t cardIndex)
            -> std::pair<std::size_t, CardProgress> {
        return {cardIndex, cards[cardIndex].Progress()};
    };
    std::function<std::pair<std::size_t, folly::sorted_vector_set<std::size_t>>(std::size_t)>
            index_cardVocableIndices = [this](std::size_t cardIndex)
            -> std::pair<std::size_t, folly::sorted_vector_set<std::size_t>> {
        return {cardIndex, cards[cardIndex].VocableIndices()};
    };
    void fillIndexMaps();
    void insertVocabularyOfCard(const CardDB::CardPtr& card);

    static auto getVocableIdsInOrder(const CardDB::CardPtr& card,
                                     const std::map<unsigned, unsigned>& vocableChoices) -> std::vector<unsigned>;
};
} // namespace sr
