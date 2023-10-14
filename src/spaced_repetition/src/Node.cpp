#include "Node.h"

#include <WalkableData.h>
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

Node::Node(std::shared_ptr<WalkableData> _walkableData,
           std::shared_ptr<node_vector> _nodes,
           size_t _cardIndex)
    : walkableData{std::move(_walkableData)}
    , nodes{std::move(_nodes)}
    , cardIndex{_cardIndex}
    , subCards{collectSubCards()}
{
    spdlog::info("Subcards size {}, for index: {}", subCards.size(), cardIndex);
}

auto Node::lowerOrder(size_t order) -> std::optional<Path>
{
    auto& cards = walkableData->Cards();
    // const auto& thisTnv = cards[cardIndex].getTimingAndVocables();
    cardsLessVocables = removeInactiveCardindices(subCards);
    if (cardsLessVocables.empty()) {
        return {};
    }
    sortCardIndices(cardsLessVocables);
    spdlog::info("order: {}, lowerOrder subCards size: {}, voc size:{}, cardId: {}",
                 order,
                 cardsLessVocables.size(),
                 cards[cardsLessVocables[0]].getTimingAndVocables().vocables.size(),
                 walkableData->Cards().id_from_index(cardsLessVocables[0]));
    for (size_t index : cardsLessVocables) {
        if (nodes->at(index).has_value()) {
            continue;
        }
        auto tempNode = nodes->at(index).emplace(walkableData, nodes, index);
        auto optionalPath = tempNode.lowerOrder(order + 1);
        if (optionalPath.has_value()) {
            paths.push_back(std::move(optionalPath.value()));
        }
    }
    return {{.cardIndex = cardsLessVocables[0]}};
}

auto Node::collectSubCards() const -> index_set
{
    index_set subCardsResult;

    // const auto& card = walkableData->Cards()[cardIndex];
    const auto& tnv = walkableData->Cards()[cardIndex].getTimingAndVocables();
    const auto& vocables = walkableData->Vocables();
    const auto& containedVocables = tnv.vocables
                                    | views::transform([&vocables](size_t index) -> const sr::VocableMeta& {
                                          return vocables[index];
                                      });
    for (const auto& vocableMeta : containedVocables) {
        ranges::copy(vocableMeta.CardIndices(), std::inserter(subCardsResult, subCardsResult.begin()));
    }
    return subCardsResult;
}

auto Node::removeInactiveCardindices(const index_set& cardIndices) -> std::vector<size_t>
{
    std::vector<size_t> result;
    auto& cards = walkableData->Cards();
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
        //     ranges::sort(tempIndices, std::less{}, [this](size_t tindex) { return walkableData->Vocables()[tindex].Progress().getRepeatRange().daysMin; });
        //     // for (size_t vocindex : cards[index].VocableIndices()) {
        //     for (size_t vocindex : tempIndices) {
        //         auto daysMin = walkableData->Vocables()[vocindex].Progress().getRepeatRange().daysMin;
        //         auto daysNormal = walkableData->Vocables()[vocindex].Progress().getRepeatRange().daysNormal;
        //         auto daysMax = walkableData->Vocables()[vocindex].Progress().getRepeatRange().daysMax;
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
    auto& cards = walkableData->Cards();
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
        if (tnv_a.vocables.size() != tnv_b.vocables.size()) {
            return preferedQuantity(tnv_a.vocables.size(), tnv_b.vocables.size());
        }
        size_t countIntersect_a = ranges::set_intersection(
                                          thisTnv.vocables, tnv_a.vocables, utl::counting_iterator{})
                                          .out.count;
        size_t countIntersect_b = ranges::set_intersection(
                                          thisTnv.vocables, tnv_b.vocables, utl::counting_iterator{})
                                          .out.count;
        return countIntersect_a > countIntersect_b;
    });
}

} // namespace sr
