#include "TreeWalker.h"

#include <ITreeWalker.h>
#include <annotation/Ease.h>
#include <bits/ranges_algo.h>
#include <fmt/format.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <srtypes.h>
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

auto ITreeWalker::createTreeWalker(std::shared_ptr<DataBase> db) -> std::unique_ptr<ITreeWalker>
{
    return std::make_unique<TreeWalker>(std::move(db));
}

TreeWalker::TreeWalker(std::shared_ptr<DataBase> _db)
    : db{std::move(_db)}
{
}

auto TreeWalker::getTodayVocables() const -> index_set
{
    const auto& cards = db->Cards();
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
    const auto& vocables = db->Vocables();
    sr::index_set todayVocables = getTodayVocables();
    while (true) {
        if (todayVocables.empty()) {
            return {};
        }
        spdlog::info("##### Vocables to study: {} #####", todayVocables.size());
        auto recency = [&vocables](size_t index) -> float { return vocables[index].Progress().recency(); };
        size_t targetVocable = *ranges::min_element(todayVocables, std::less{}, recency);

        auto cardId = db->Vocables()[targetVocable].getNextTriggerCard(db);
        auto cardIndex = db->Cards().index_at_id(cardId);
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
        const VocableMeta& vocable = db->Vocables()[failedVoc];
        if (vocable.Progress().pauseTimeOver()) {
            spdlog::info("pause time over for: {}", failedVoc);
            failedVocables.erase(failedVoc);
            CardId triggerCard = vocable.getNextTriggerCard(db);
            size_t triggerCardIndex = db->Cards().index_at_id(triggerCard);
            ignoreCards->erase(triggerCardIndex);
            cardRemoved = true;
        }
    }
    std::set<CardId> ignoreCardSet;
    ranges::transform(*ignoreCards,
                      std::inserter(ignoreCardSet, ignoreCardSet.begin()),
                      [this](size_t index) { return db->Cards().id_from_index(index); });
    spdlog::info("Ignore CardIds: [{}]", fmt::join(ignoreCardSet, ", "));
    return cardRemoved;
}

auto TreeWalker::getFailedVocIgnoreCardIndices() const -> std::shared_ptr<index_set>
{
    auto result = std::make_shared<index_set>();
    for (size_t failedVoc : failedVocables) {
        const auto& cardIndices = db->Vocables()[failedVoc].CardIndices();
        ranges::copy(cardIndices, std::inserter(*result, result->begin()));
    }
    return result;
}

void TreeWalker::addNextVocableToIgnoreCardIndices(size_t nextVocable, std::shared_ptr<index_set>& ignoreCardIndices)
{
    const auto& cardIndices = db->Vocables()[nextVocable].CardIndices();
    ranges::copy(cardIndices, std::inserter(*ignoreCardIndices, ignoreCardIndices->begin()));
}

auto TreeWalker::getNextCardChoice(std::optional<CardId> preferedCardId) -> CardMeta&
{
    size_t activeCardIndex{};
    if (not preferedCardId.has_value()) {
        activeCardIndex = getNextTargetCard().value_or(0);
    } else {
        auto optional_index = db->Cards().optional_index(preferedCardId.value());
        activeCardIndex = optional_index.value_or(0);
        if (not optional_index.has_value()) {
            spdlog::error("prefered card Id could not be found in cards index_map!");
        }
    }
    currentCardIndex = activeCardIndex;
    return db->Cards()[activeCardIndex];
}

auto TreeWalker::getLastCard() -> CardMeta&
{
    return db->Cards()[currentCardIndex];
}

void TreeWalker::setEaseLastCard(const Id_Ease_vt& id_ease)
{
    CardId currentCardId = db->Cards().id_from_index(currentCardIndex);
    for (auto [tmpVocId, ease] : id_ease) {
        auto vocId = db->unmapVocableChoice(tmpVocId);
        // spdlog::warn("begin id: {}", vocId);
        db->setEaseVocable(vocId, ease);
        db->triggerVocable(vocId, currentCardId);
        // spdlog::warn("intDay: {}, daysMin {}, daysNormal: {}, daysMax: {} id: {}, ease: {}",
        //              db->Vocables().at_id(vocId).second.Progress().IntervalDay(),
        //              db->Vocables().at_id(vocId).second.Progress().getRepeatRange().daysMin,
        //              db->Vocables().at_id(vocId).second.Progress().getRepeatRange().daysNormal,
        //              db->Vocables().at_id(vocId).second.Progress().getRepeatRange().daysMax,
        //              vocId, static_cast<unsigned>(ease.easeVal));
        db->resetCardsContainingVocable(vocId);
        if (ease.easeVal == EaseVal::again) {
            size_t vocableIndex = db->Vocables().index_at_id(vocId);
            failedVocables.insert(vocableIndex);
        }
    }
}

auto TreeWalker::createTree(size_t targetVocableIndex, std::shared_ptr<index_set> ignoreCardIndices) const -> Tree
{
    auto cardId = db->Vocables()[targetVocableIndex].getNextTriggerCard(db);
    auto cardIndex = db->Cards().index_at_id(cardId);

    spdlog::info("TargetVocable: {}, cardSize: {}", targetVocableIndex,
                 db->Cards()[cardIndex].getTimingAndVocables().vocables.size());
    auto resultTree = Tree{db, targetVocableIndex, cardIndex, std::move(ignoreCardIndices)};
    resultTree.build();
    return resultTree;
}

} // namespace sr
