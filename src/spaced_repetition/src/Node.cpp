#include "Node.h"

#include "Path.h"

#include <DataBase.h>
#include <spdlog/spdlog.h>
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
           size_t _cardIndex,
           std::shared_ptr<index_set> _ignoreCardIndices)
    : db{std::move(_db)}
    , nodes{std::move(_nodes)}
    , cardIndex{_cardIndex}
    , ignoreCardIndices{std::move(_ignoreCardIndices)}
    , subCards{collectSubCards()}
{
    // spdlog::info("nVocables: {} nSubcards {}, for cardId: {}",
    //              db->Cards()[cardIndex].getTimingAndVocables().vocables.size(),
    //              subCards.size(),
    //              db->Cards().id_from_index(cardIndex));
}

void Node::tighten()
{
    for (Path& path : paths) {
        traverseAndTighten(path);
    }
}

void Node::traverseAndTighten(Path& path)
{
}

auto Node::lowerOrder(size_t order) -> size_t
{
    size_t nextOrder = order;
    auto& cards = db->Cards();
    const auto& thisTnv = cards[cardIndex].getTimingAndVocables();
    if (thisTnv.vocables.size() <= s_stopBreakDown) {
        return nextOrder;
    }

    cardsLessVocables = removeInactiveCardindices(subCards);
    sortCardIndices(cardsLessVocables);
    // spdlog::info("order: {}, lowerOrder subCards size: {}, voc size:{}, cardId: {}",
    //              order,
    //              cardsLessVocables.size(),
    //              cards[cardIndex].getTimingAndVocables().vocables.size(),
    //              db->Cards().id_from_index(cardIndex));
    for (size_t index : cardsLessVocables) {
        if ((*nodes)[index].has_value()) {
            continue;
        }
        (*nodes)[index].emplace(db, nodes, index, ignoreCardIndices);
        Path path{};
        path.cardIndex = index;
        paths.push_back(path);
    }
    for (Path& path : paths) {
        auto& optionalNode = (*nodes)[path.cardIndex];
        if (not optionalNode.has_value()) {
            break;
        }
        auto& tempNode = optionalNode.value();
        nextOrder = std::max(nextOrder, tempNode.lowerOrder(order + 1));
    }
    // spdlog::info("nextOrder: {}, cardId: {}",
    //              nextOrder,
    //              db->Cards().id_from_index(cardIndex));
    return nextOrder;
}

auto Node::Paths() const -> const std::vector<Path>&
{
    // if (paths.empty()) {
    //     spdlog::info("empty cardId: {}",
    //                  db->Cards().id_from_index(cardIndex));
    // }
    return paths;
}

auto Node::collectSubCards() const -> index_set
{
    index_set subCardsResult;

    // const auto& card = db->Cards()[cardIndex];
    const auto& tnv = db->Cards()[cardIndex].getTimingAndVocables();
    const auto& vocables = db->Vocables();
    const auto& containedVocables = tnv.vocables
                                    | views::transform([&vocables](size_t index) -> const sr::VocableMeta& {
                                          return vocables[index];
                                      });
    for (const auto& vocableMeta : containedVocables) {
        // ranges::copy(vocableMeta.CardIndices(), std::inserter(subCardsResult, subCardsResult.begin()));
        ranges::set_difference(vocableMeta.CardIndices(),
                               *ignoreCardIndices,
                               std::inserter(subCardsResult, subCardsResult.begin()));
    }

    return subCardsResult;
}

auto Node::removeInactiveCardindices(const index_set& cardIndices) -> std::vector<size_t>
{
    std::vector<size_t> result;
    auto& cards = db->Cards();
    const auto& thisTnv = cards[cardIndex].getTimingAndVocables();
    ranges::copy_if(cardIndices, std::back_inserter(result), [&cards, &thisTnv](size_t index) -> bool {
        const auto& tnv = cards[index].getTimingAndVocables();
        // if (tnv.vocables.empty()) {
        //     auto& card = cards[index];
        //     card.resetTimingAndVocables();
        //     const auto& tnvp = card.getTimingAndVocables();
        //
        //     spdlog::error("empty card has {} vocables!, tnvp size: {} timing: {}",
        //                   card.VocableIndices().size(),
        //                   tnvp.vocables.size(),
        //                   tnvp.timing);
        //     std::vector<size_t> tempIndices;
        //     ranges::copy(card.VocableIndices(), std::back_inserter(tempIndices));
        //     ranges::sort(tempIndices, std::less{}, [this](size_t tindex) { return db->Vocables()[tindex].Progress().getRepeatRange().daysMin; });
        //     // for (size_t vocindex : cards[index].VocableIndices()) {
        //     for (size_t vocindex : tempIndices) {
        //         auto daysMin = db->Vocables()[vocindex].Progress().getRepeatRange().daysMin;
        //         auto daysNormal = db->Vocables()[vocindex].Progress().getRepeatRange().daysNormal;
        //         auto daysMax = db->Vocables()[vocindex].Progress().getRepeatRange().daysMax;
        //         spdlog::info("   mindays {}, ndays {}, maxdays {}", daysMin, daysNormal, daysMax);
        //     }
        // }
        return tnv.timing <= 0
               && not tnv.vocables.empty()
               && tnv.vocables.size() < thisTnv.vocables.size();
    });
    return result;
}

void Node::sortCardIndices(std::vector<size_t>& cardIndices)
{
    auto& cards = db->Cards();
    const auto& thisTnv = cards[cardIndex].getTimingAndVocables();
    const auto preferedQuantity = [](size_t a, size_t b) -> bool {
        const std::array quantity = {4, 3, 5, 2, 6};
        const auto* a_it = ranges::find(quantity, a);
        const auto* b_it = ranges::find(quantity, b);
        if (a_it != b_it) {
            return a_it < b_it;
        }
        return a < b;
    };
    ranges::sort(cardIndices, [&, this](size_t index_a, size_t index_b) -> bool {
        const auto& tnv_a = cards[index_a].getTimingAndVocables();
        const auto& tnv_b = cards[index_b].getTimingAndVocables();
        size_t countIntersect_a = ranges::set_intersection(
                                          thisTnv.vocables, tnv_a.vocables, utl::counting_iterator{})
                                          .out.count;
        size_t countIntersect_b = ranges::set_intersection(
                                          thisTnv.vocables, tnv_b.vocables, utl::counting_iterator{})
                                          .out.count;
        if (tnv_a.vocables.size() != tnv_b.vocables.size()) {
            return preferedQuantity(tnv_a.vocables.size(), tnv_b.vocables.size());
        }
        // if (countIntersect_a != countIntersect_b) {
        return countIntersect_a > countIntersect_b;
        // }
    });
}

} // namespace sr
