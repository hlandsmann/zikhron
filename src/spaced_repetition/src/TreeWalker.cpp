#include "TreeWalker.h"

#include <ITreeWalker.h>
#include <WalkableData.h>
#include <annotation/Ease.h>
#include <bits/ranges_algo.h>
#include <fmt/format.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <utils/counting_iterator.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <utility>

#include <sys/types.h>

namespace ranges = std::ranges;
namespace views = std::ranges::views;

namespace sr {

auto ITreeWalker::createTreeWalker(std::shared_ptr<WalkableData> walkableData) -> std::unique_ptr<ITreeWalker>
{
    return std::make_unique<TreeWalker>(std::move(walkableData));
}

TreeWalker::TreeWalker(std::shared_ptr<WalkableData> _walkableData)
    : walkableData{std::move(_walkableData)}
{
    // walk(walkableData);
    // createTree();
}

auto TreeWalker::getTodayVocables() const -> index_set
{
    const auto& cards = walkableData->Cards();
    sr::index_set todayVocables;
    for (const auto& card : cards) {
        auto tnv = card.getTimingAndVocables();
        if (tnv.timing <= 0) {
            todayVocables.insert(tnv.vocables.begin(), tnv.vocables.end());
        }
    }
    return todayVocables;
}

auto TreeWalker::getNextTargetVocable(const std::shared_ptr<index_set>& ignoreCards) const -> std::optional<size_t>
{
    const auto& vocables = walkableData->Vocables();
    sr::index_set todayVocables = getTodayVocables();
    while (true) {
        if (todayVocables.empty()) {
            return {};
        }
        spdlog::info("##### Vocables to study: {} #####", todayVocables.size());
        auto recency = [&vocables](size_t index) -> float { return vocables[index].Progress().recency(); };
        size_t targetVocable = *ranges::min_element(todayVocables, std::less{}, recency);

        auto cardId = walkableData->Vocables()[targetVocable].getNextTriggerCard(walkableData);
        auto cardIndex = walkableData->Cards().index_at_id(cardId);
        if (not ignoreCards->contains(cardIndex)) {
            return {targetVocable};
        }
        todayVocables.erase(targetVocable);
        spdlog::info("Dropping targetVocableIndex: {} for CardId: {}", targetVocable, cardId);
    }
}

auto TreeWalker::getNextTargetCard() -> std::optional<size_t>
{
    std::optional<size_t> result = std::nullopt;
    auto ignoreCardIndices = getFailedVocIgnoreCardIndices();

    if (not removeRepeatNowCardIndex(ignoreCardIndices)) {
        result = getFailedVocableCleanTreeCardIndex(ignoreCardIndices);
    }
    if (result.has_value()) {
        return result;
    }
    std::optional<size_t> optionalNextVocable = getNextTargetVocable(ignoreCardIndices);
    if (not optionalNextVocable.has_value()) {
        return {};
    }
    size_t nextVocable = optionalNextVocable.value();

    addNextVocableToIgnoreCardIndices(nextVocable, ignoreCardIndices);
    auto tree = createTree(nextVocable, ignoreCardIndices);
    auto optionalSubCard = tree.getNodeCardIndex();
    if (optionalSubCard.has_value()) {
        result = optionalSubCard;
    } else {
        result = tree.getRoot();
    }
    return result;
}

auto TreeWalker::getFailedVocableCleanTreeCardIndex(const std::shared_ptr<index_set>& ignoreCardIndices) -> std::optional<size_t>
{
    std::optional<size_t> result = std::nullopt;
    for (size_t failedVoc : failedVocables) {
        if (not vocableIndex_tree.contains(failedVoc)) {
            vocableIndex_tree[failedVoc] = createTree(failedVoc, ignoreCardIndices);
        }
        auto& optionalTree = vocableIndex_tree[failedVoc];
        if (not optionalTree.has_value()) {
            continue;
        }
        if (result = optionalTree->getNodeCardIndex(); result.has_value()) {
            vocableIndex_tree.erase(failedVoc);
            return result;
        }
    }
    return result;
}

auto TreeWalker::removeRepeatNowCardIndex(const std::shared_ptr<index_set>& ignoreCards) -> bool
{
    bool cardRemoved = false;
    for (size_t failedVoc : failedVocables) {
        const VocableMeta& vocable = walkableData->Vocables()[failedVoc];
        if (vocable.Progress().pauseTimeOver()) {
            spdlog::info("pause time over for: {}", failedVoc);
            failedVocables.erase(failedVoc);
            CardId triggerCard = vocable.getNextTriggerCard(walkableData);
            size_t triggerCardIndex = walkableData->Cards().index_at_id(triggerCard);
            ignoreCards->erase(triggerCardIndex);
            cardRemoved = true;
        }
    }
    std::set<CardId> ignoreCardSet;
    ranges::transform(*ignoreCards,
                      std::inserter(ignoreCardSet, ignoreCardSet.begin()),
                      [this](size_t index) { return walkableData->Cards().id_from_index(index); });
    spdlog::info("Ignore CardIds: [{}]", fmt::join(ignoreCardSet, ", "));
    return cardRemoved;
}

auto TreeWalker::getFailedVocIgnoreCardIndices() const -> std::shared_ptr<index_set>
{
    auto result = std::make_shared<index_set>();
    for (size_t failedVoc : failedVocables) {
        const auto& cardIndices = walkableData->Vocables()[failedVoc].CardIndices();
        ranges::copy(cardIndices, std::inserter(*result, result->begin()));
    }
    return result;
}

void TreeWalker::addNextVocableToIgnoreCardIndices(size_t nextVocable, std::shared_ptr<index_set>& ignoreCardIndices)
{
    const auto& cardIndices = walkableData->Vocables()[nextVocable].CardIndices();
    ranges::copy(cardIndices, std::inserter(*ignoreCardIndices, ignoreCardIndices->begin()));
}

auto TreeWalker::getNextCardChoice(std::optional<CardId> preferedCardId) -> CardInformation
{
    size_t activeCardIndex{};
    if (not preferedCardId.has_value()) {
        auto optionalCardIndex = getNextTargetCard();
        if (not optionalCardIndex.has_value()) {
            return {};
        }
        activeCardIndex = optionalCardIndex.value();
    } else {
        auto optional_index = walkableData->Cards().optional_index(preferedCardId.value());
        if (optional_index.has_value()) {
            activeCardIndex = optional_index.value();
        } else {
            spdlog::error("prefered card Id could not be found in cards index_map!");
            return {};
        }
    }
    auto card = walkableData->getCardCopy(activeCardIndex);
    currentCardIndex = activeCardIndex;
    return {std::move(card),
            walkableData->getVocableIdsInOrder(activeCardIndex),
            walkableData->getRelevantEase(activeCardIndex)};
}

void TreeWalker::setEaseLastCard(const Id_Ease_vt& id_ease)
{
    CardId currentCardId = walkableData->Cards().id_from_index(currentCardIndex);
    for (auto [vocId, ease] : id_ease) {
        // spdlog::warn("begin id: {}", vocId);
        walkableData->setEaseVocable(vocId, ease);
        walkableData->triggerVocable(vocId, currentCardId);
        // spdlog::warn("intDay: {}, daysMin {}, daysNormal: {}, daysMax: {} id: {}, ease: {}",
        //              walkableData->Vocables().at_id(vocId).second.Progress().IntervalDay(),
        //              walkableData->Vocables().at_id(vocId).second.Progress().getRepeatRange().daysMin,
        //              walkableData->Vocables().at_id(vocId).second.Progress().getRepeatRange().daysNormal,
        //              walkableData->Vocables().at_id(vocId).second.Progress().getRepeatRange().daysMax,
        //              vocId, static_cast<unsigned>(ease.easeVal));
        walkableData->resetCardsContainingVocable(vocId);
        if (ease.easeVal == EaseVal::again) {
            size_t vocableIndex = walkableData->Vocables().index_at_id(vocId);
            failedVocables.insert(vocableIndex);
        }
    }
}

auto TreeWalker::createTree(size_t targetVocableIndex, std::shared_ptr<index_set> ignoreCardIndices) const -> Tree
{
    auto cardId = walkableData->Vocables()[targetVocableIndex].getNextTriggerCard(walkableData);
    auto cardIndex = walkableData->Cards().index_at_id(cardId);

    spdlog::info("TargetVocable: {}, cardSize: {}", targetVocableIndex,
                 walkableData->Cards()[cardIndex].getTimingAndVocables().vocables.size());
    auto resultTree = Tree{walkableData, targetVocableIndex, cardIndex, std::move(ignoreCardIndices)};
    resultTree.build();
    return resultTree;
}

void TreeWalker::saveProgress() const
{
    walkableData->saveProgress();
}
} // namespace sr
