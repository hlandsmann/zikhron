#include "Node.h"

#include "Path.h"

#include <CardContent.h>
#include <DataBase.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <srtypes.h>
#include <utils/counting_iterator.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

namespace ranges = std::ranges;
namespace views = std::ranges::views;

namespace sr {

Node::Node(std::shared_ptr<DataBase> _db,
           std::shared_ptr<node_vector> _nodes,
           CardId _cardId,
           std::shared_ptr<cardId_set> _ignoreCardIds)
    : db{std::move(_db)}
    , weakNodes{std::move(_nodes)}
    , nodeCardId{_cardId}
    , ignoreCardIds{std::move(_ignoreCardIds)}
    , subCards{collectSubCards()}
{}

void Node::tighten()
{
    for (Path& path : paths) {
        traverseAndTighten(path);
    }
}

void Node::traverseAndTighten(Path& /* path */)
{
}

auto Node::lowerOrder(size_t order) -> size_t
{
    auto nodes = weakNodes.lock();
    if (!nodes) {
        return {};
    }
    size_t nextOrder = order;
    const auto& cards = db->MetaCards();
    const auto& thisTnv = cards.at(nodeCardId).getTimingAndVocables(CardContent::pulled);
    if (thisTnv.vocables.size() <= s_stopBreakDown) {
        return nextOrder;
    }

    auto cardsLessVocables = removeInactiveCardIds(subCards);
    sortCardIds(cardsLessVocables);

    int loopLimiter = 0;
    for (CardId index : cardsLessVocables) {
        if ((*nodes)[index].has_value()) {
            continue;
        }
        (*nodes)[index].emplace(db, nodes, index, ignoreCardIds);
        Path path{};
        path.cardId = index;
        paths.push_back(path);
        if (loopLimiter++ > 500) {
            spdlog::warn("Break out of loop / cancel calculation");
            break;
        }
    }
    for (Path& path : paths) {
        auto& optionalNode = (*nodes)[path.cardId];
        if (not optionalNode.has_value()) {
            break;
        }
        auto& tempNode = optionalNode.value();
        nextOrder = std::max(nextOrder, tempNode.lowerOrder(order + 1));
    }
    return nextOrder;
}

auto Node::Paths() const -> const std::vector<Path>&
{
    return paths;
}

auto Node::collectSubCards() const -> cardId_set
{
    cardId_set subCardsResult;

    const auto& tnv = db->MetaCards().at(nodeCardId).getTimingAndVocables(CardContent::pulled);
    const auto& vocables = db->Vocables();
    const auto& containedVocables = tnv.vocables
                                    | views::transform([&vocables](size_t index) -> const sr::VocableMeta& {
                                          return vocables[index];
                                      });
    for (const auto& vocableMeta : containedVocables) {
        ranges::set_difference(vocableMeta.CardIds(),
                               *ignoreCardIds,
                               std::inserter(subCardsResult, subCardsResult.begin()));
    }

    return subCardsResult;
}

auto Node::removeInactiveCardIds(const cardId_set& cardIds) -> std::vector<CardId>
{
    std::vector<CardId> result;
    const auto& cards = db->MetaCards();
    const auto& thisTnv = cards.at(nodeCardId).getTimingAndVocables(CardContent::pulled);
    ranges::copy_if(cardIds, std::back_inserter(result), [&cards, &thisTnv](CardId cardId) -> bool {
        const auto& tnv = cards.at(cardId).getTimingAndVocables(CardContent::pulled);
        return tnv.timing <= 0
               && not tnv.vocables.empty()
               && tnv.vocables.size() < thisTnv.vocables.size();
    });
    return result;
}

void Node::sortCardIds(std::vector<CardId>& cardIds)
{
    const auto& cards = db->MetaCards();
    const auto& thisTnv = cards.at(nodeCardId).getTimingAndVocables(CardContent::pulled);
    const auto preferedQuantity = [](size_t a, size_t b) -> bool {
        const std::array quantity = {4, 3, 2, 5, 1, 6};
        const auto* a_it = ranges::find(quantity, a);
        const auto* b_it = ranges::find(quantity, b);
        if (a_it != b_it) {
            return a_it < b_it;
        }
        return a < b;
    };
    const auto triggerValue = [&](CardId cardId) -> std::size_t {
        const auto& tnv = cards.at(cardId).getTimingAndVocables(CardContent::pulled);
        index_set intersect;
        ranges::set_intersection(thisTnv.vocables, tnv.vocables, std::inserter(intersect, intersect.begin()));
        std::size_t value = 0;
        for (const auto& index : intersect) {
            value += db->Vocables()[index].triggerValue(cardId);
        }
        return value;
    };
    ranges::sort(cardIds, [&](CardId cardId_a, CardId cardId_b) -> bool {
        const auto& tnv_a = cards.at(cardId_a).getTimingAndVocables(CardContent::pulled);
        const auto& tnv_b = cards.at(cardId_b).getTimingAndVocables(CardContent::pulled);
        size_t countIntersect_a = ranges::set_intersection(
                                          thisTnv.vocables, tnv_a.vocables, utl::counting_iterator{})
                                          .out.count;
        size_t countIntersect_b = ranges::set_intersection(
                                          thisTnv.vocables, tnv_b.vocables, utl::counting_iterator{})
                                          .out.count;
        if (tnv_a.vocables.size() != tnv_b.vocables.size()) {
            return preferedQuantity(tnv_a.vocables.size(), tnv_b.vocables.size());
        }
        const std::size_t triggerValue_a = triggerValue(cardId_a);
        const std::size_t triggerValue_b = triggerValue(cardId_b);
        if (triggerValue_a != triggerValue_b) {
            // spdlog::info("TriggerValueA: {}, triggerValueB: {}", triggerValue_a, triggerValue_b);
            return triggerValue_a > triggerValue_b;
        }
        return countIntersect_a > countIntersect_b;
    });
}

} // namespace sr
