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
#include <optional>
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

struct TimingAndVocables
{
    int timing{};
    index_set vocables{};
};

struct CardMeta
{
    CardMeta(folly::sorted_vector_set<std::size_t> _vocableIndices,
             std::reference_wrapper<utl::index_map<VocableId, VocableMeta>> _refVocables);
    [[nodiscard]] auto VocableIndices() const -> const folly::sorted_vector_set<std::size_t>&;
    void vocableIndices_insert(std::size_t vocableIndex);
    [[nodiscard]] auto getTimingAndVocables(bool pull = false) const -> const TimingAndVocables&;
    void resetTimingAndVocables();

private:
    [[nodiscard]] auto generateTimingAndVocables(bool pull) const -> TimingAndVocables;
    folly::sorted_vector_set<std::size_t> vocableIndices;
    mutable std::optional<TimingAndVocables> timingAndVocables;
    mutable std::optional<TimingAndVocables> timingAndVocablesPulled;
    std::reference_wrapper<utl::index_map<VocableId, VocableMeta>> refVocables;
};

class WalkableData
{
public:
    WalkableData(std::shared_ptr<zikhron::Config> config);
    [[nodiscard]] auto Vocables() const -> const utl::index_map<VocableId, VocableMeta>&;
    [[nodiscard]] auto Cards() -> utl::index_map<CardId, CardMeta>&;
    [[nodiscard]] auto getCardCopy(size_t cardIndex) const -> CardDB::CardPtr;

    // [[nodiscard]] auto timingAndVocables(const CardMeta& card, bool pull) const -> TimingAndVocables;
    // [[nodiscard]] auto timingAndVocables(const CardMeta& card) const -> TimingAndVocables;
    // [[nodiscard]] auto timingAndVocables(size_t cardIndex, bool pull) const -> TimingAndVocables;
    // [[nodiscard]] auto timingAndVocables(size_t cardIndex) const -> TimingAndVocables;

    [[nodiscard]] auto getActiveVocables(size_t cardIndex) -> std::set<VocableId>;

    [[nodiscard]] auto getVocableIdsInOrder(size_t cardIndex) const -> std::vector<VocableId>;
    [[nodiscard]] auto getRelevantEase(size_t cardIndex) -> std::map<VocableId, Ease>;
    void setEaseVocable(VocableId, Ease);
    void resetCardsContainingVocable(VocableId vocId);
    void saveProgress() const;

private:
    void fillIndexMaps();
    void insertVocabularyOfCard(const CardDB::CardPtr& card);
    static auto getVocableIdsInOrder(const CardDB::CardPtr& card,
                                     const std::map<unsigned, unsigned>& vocableChoices) -> std::vector<VocableId>;
    [[nodiscard]] auto generateVocableIdProgressMap() const -> std::map<VocableId, VocableProgress>;

    DataBase db;
    utl::index_map<VocableId, VocableMeta> vocables;
    utl::index_map<CardId, CardMeta> cards;

    std::function<VocableProgress(std::size_t)>
            vocable_progress = [this](std::size_t vocableIndex) { return vocables[vocableIndex].Progress(); };
    std::function<folly::sorted_vector_set<std::size_t>(std::size_t)>
            vocable_cardIndices = [this](std::size_t vocableIndex) { return vocables[vocableIndex].CardIndices(); };
    // std::function<CardProgress(std::size_t)>
    //         card_progress = [this](std::size_t cardIndex) { return cards[cardIndex].Progress(); };
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
    // std::function<std::pair<std::size_t, CardProgress>(std::size_t)>
    //         index_cardProgress = [this](std::size_t cardIndex)
    //         -> std::pair<std::size_t, CardProgress> {
    //     return {cardIndex, cards[cardIndex].Progress()};
    // };
    std::function<std::pair<std::size_t, folly::sorted_vector_set<std::size_t>>(std::size_t)>
            index_cardVocableIndices = [this](std::size_t cardIndex)
            -> std::pair<std::size_t, folly::sorted_vector_set<std::size_t>> {
        return {cardIndex, cards[cardIndex].VocableIndices()};
    };
};
} // namespace sr
