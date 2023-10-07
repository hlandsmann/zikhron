#pragma once
#include "CardProgress.h"
#include "DataBase.h"
#include "VocableProgress.h"

#include <annotation/Card.h>
#include <annotation/Ease.h>
#include <annotation/ZH_Annotator.h>
#include <folly/sorted_vector_types.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
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
    [[nodiscard]] auto Progress() const -> const VocableProgress&;
    [[nodiscard]] auto CardIndices() const -> const folly::sorted_vector_set<std::size_t>&;
    void advanceByEase(Ease);
    void cardIndices_insert(std::size_t cardIndex);

private:
    VocableProgress progress;
    folly::sorted_vector_set<std::size_t> cardIndices;
    ZH_Annotator::ZH_dicItemVec dicItemVec;
};

struct CardMeta
{
    CardMeta(CardProgress progress, folly::sorted_vector_set<std::size_t> _vocableIndices);
    [[nodiscard]] auto Progress() const -> const CardProgress&;
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
    [[nodiscard]] auto Vocables() const -> const utl::index_map<VocableId, VocableMeta>&;
    [[nodiscard]] auto Cards() const -> const utl::index_map<CardId, CardMeta>&;
    [[nodiscard]] auto getCardCopy(size_t cardIndex) const -> CardDB::CardPtr;

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
    [[nodiscard]] auto getActiveVocables(size_t cardIndex) const -> std::set<VocableId>;

    [[nodiscard]] auto getVocableIdsInOrder(size_t cardIndex) const -> std::vector<VocableId>;
    [[nodiscard]] auto getRelevantEase(size_t cardIndex) const -> std::map<VocableId, Ease>;
    void setEaseVocable(VocableId, Ease);

private:
    void fillIndexMaps();
    void insertVocabularyOfCard(const CardDB::CardPtr& card);
    [[nodiscard]] auto generateVocableIdProgressMap() const -> std::map<VocableId, VocableProgress>;

    static auto getVocableIdsInOrder(const CardDB::CardPtr& card,
                                     const std::map<unsigned, unsigned>& vocableChoices) -> std::vector<VocableId>;

    DataBase db;
    utl::index_map<VocableId, VocableMeta> vocables;
    utl::index_map<CardId, CardMeta> cards;

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
};
} // namespace sr
