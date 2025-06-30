#include "TreeWalker.h"

#include <CardContent.h>
#include <ITreeWalker.h>
#include <annotation/Ease.h>
#include <bits/ranges_algo.h>
#include <database/CbdFwd.h>
#include <database/SpacedRepetitionData.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <srtypes.h>
#include <utils/counting_iterator.h>
#include <utils/format.h>

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

void TreeWalker::updateFailedVocables()
{
    numberOfFailedVocables = 0;
    failedVocables.clear();
    const auto& vocables = db->Vocables();
    for (const auto& [index, vocableMeta] : views::enumerate(vocables)) {
        if (!vocableMeta.CardIds().empty()
            && vocableMeta.SpacedRepetitionData()->enabled
            && vocableMeta.SpacedRepetitionData()->state != database::StudyState::review) {
            failedVocables.insert(static_cast<std::size_t>(index));
            numberOfFailedVocables++;
        }
    }
}

auto TreeWalker::getTodayVocables() const -> index_set
{
    const auto& cards = db->MetaCards();
    sr::index_set todayVocables;
    for (const auto& [_, card] : cards) {
        auto tnv = card.getTimingAndVocables();
        if (tnv.timing <= 0) {
            todayVocables.insert(tnv.vocables.begin(), tnv.vocables.end());
        }
        // if (_ == static_cast<CardId>(1887)) {
        //     spdlog::critical("timing: {}, vocsize:{}", tnv.timing, tnv.vocables.size());
        //     spdlog::critical("token: {}", fmt::join(card.getCard()->getTokens()," "));
        // }
    }

    numberOfTodayVocables = static_cast<unsigned>(todayVocables.size());
    return todayVocables;
}

auto TreeWalker::getNextTargetVocable(const std::shared_ptr<cardId_set>& ignoreCards) const -> std::optional<size_t>
{
    const auto& vocables = db->Vocables();
    sr::index_set todayVocables = getTodayVocables();
    while (true) {
        if (todayVocables.empty()) {
            return {};
        }
        spdlog::info("##### Vocables to study: {} #####", todayVocables.size());
        auto recency = [&vocables](size_t index) -> double {
            const auto& srd = *vocables[index].SpacedRepetitionData();
            return srd.recency();

            // return vocables[index].SpacedRepetitionData()->stability;
        };
        size_t targetVocable = *ranges::min_element(todayVocables, std::less{}, recency);

        const auto& voc = vocables[targetVocable];
        spdlog::info("TargetVoc: {}, recency: {}", targetVocable, voc.SpacedRepetitionData()->recency());

        auto cardId = db->Vocables()[targetVocable].getNextTriggerCard();
        if (not ignoreCards->contains(cardId)) {
            return {targetVocable};
        }
        todayVocables.erase(targetVocable);
        spdlog::info("Dropping targetVocableIndex: {} for CardId: {}", targetVocable, cardId);
    }
}

auto TreeWalker::getNextTargetCard() -> std::optional<CardId>
{
    std::optional<CardId> result = std::nullopt;
    auto ignoreCardIds = getFailedVocIgnoreCardIds();

    if (not removeRepeatNowCardId(ignoreCardIds)) {
        result = getFailedVocableCleanTreeCardId(ignoreCardIds);
    }
    if (result.has_value()) {
        return result;
    }
    std::optional<size_t> optionalNextVocable = getNextTargetVocable(ignoreCardIds);
    if (not optionalNextVocable.has_value()) {
        return {};
    }
    size_t nextVocable = optionalNextVocable.value();

    addNextVocableToIgnoreCardIds(nextVocable, ignoreCardIds);
    auto tree = createTree(nextVocable, ignoreCardIds);
    auto optionalSubCard = tree.getNodeCardId();
    if (optionalSubCard.has_value()) {
        result = optionalSubCard;
    } else {
        result = tree.getRoot();
    }
    return result;
}

auto TreeWalker::getFailedVocableCleanTreeCardId(const std::shared_ptr<cardId_set>& ignoreCardIds) -> std::optional<CardId>
{
    std::optional<CardId> result = std::nullopt;
    for (size_t failedVoc : failedVocables) {
        if (not vocableIndex_tree.contains(failedVoc)) {
            vocableIndex_tree[failedVoc] = createTree(failedVoc, ignoreCardIds);
        }
        auto& optionalTree = vocableIndex_tree[failedVoc];
        if (not optionalTree.has_value()) {
            continue;
        }
        if (result = optionalTree->getNodeCardId(); result.has_value()) {
            vocableIndex_tree.erase(failedVoc);
            return result;
        }
    }
    return result;
}

auto TreeWalker::removeRepeatNowCardId(const std::shared_ptr<cardId_set>& ignoreCards) -> bool
{
    bool cardRemoved = false;
    const auto failedVocablesTmp = failedVocables;
    for (size_t failedVoc : failedVocablesTmp) {
        const VocableMeta& vocable = db->Vocables()[failedVoc];
        if (vocable.SpacedRepetitionData()->pauseTimeOver()) {
            spdlog::info("pause time over for: {}", failedVoc);
            failedVocables.erase(failedVoc);

            CardId triggerCard = vocable.getNextTriggerCard();
            ignoreCards->erase(triggerCard);
            cardRemoved = true;
        }
    }
    spdlog::info("Ignore CardIds: [{}]", fmt::join(*ignoreCards, ", "));
    return cardRemoved;
}

auto TreeWalker::getFailedVocIgnoreCardIds() const -> std::shared_ptr<cardId_set>
{
    auto result = std::make_shared<cardId_set>();
    for (size_t failedVoc : failedVocables) {
        const auto& cardIds = db->Vocables()[failedVoc].CardIds();
        ranges::copy(cardIds, std::inserter(*result, result->begin()));
    }
    return result;
}

void TreeWalker::addNextVocableToIgnoreCardIds(size_t nextVocable, std::shared_ptr<cardId_set>& ignoreCardIds)
{
    const auto& cardIds = db->Vocables()[nextVocable].CardIds();
    ranges::copy(cardIds, std::inserter(*ignoreCardIds, ignoreCardIds->begin()));
}

auto TreeWalker::getNextCardChoice() -> const CardMeta&
{
    updateFailedVocables();
    CardId cardId = getNextTargetCard().value_or(CardId{});
    if (!db->MetaCards().contains(cardId)) {
        return db->MetaCards().begin()->second;
    }
    return db->MetaCards().at(cardId);
}

auto TreeWalker::getNumberOfFailedVocables() const -> unsigned
{
    return numberOfFailedVocables;
}

auto TreeWalker::getNumberOfTodayVocables() const -> unsigned
{
    return numberOfTodayVocables;
}

// void TreeWalker::setEaseForCard(database::CardPtr card, const Id_Ease_vt& id_ease)
// {
//     auto cardId = card->getCardId();
//     if (!db->cardExists(cardId)) {
//         db->addCard(card);
//     }
//     for (auto [vocId, ease] : id_ease) {
//         db->setEaseVocable(vocId, ease);
//         db->triggerVocable(vocId, cardId);
//         db->resetCardsContainingVocable(vocId);
//         if (ease.easeVal == EaseVal::again) {
//             size_t vocableIndex = db->Vocables().index_at_id(vocId);
//             failedVocables.insert(vocableIndex);
//         }
//     }
// }

auto TreeWalker::createTree(size_t targetVocableIndex, std::shared_ptr<cardId_set> ignoreCardIds) const -> Tree
{
    auto cardId = db->Vocables()[targetVocableIndex].getNextTriggerCard();

    spdlog::info("TargetVocable: {}, cardSize: {}", targetVocableIndex,
                 db->MetaCards().at(cardId).getTimingAndVocables(CardContent::pulled).vocables.size());
    auto resultTree = Tree{db, targetVocableIndex, cardId, std::move(ignoreCardIds)};
    resultTree.build();
    return resultTree;
}

} // namespace sr
