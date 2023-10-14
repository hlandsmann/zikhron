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
    spdlog::info("Subcards size {}", subCards.size());
}

auto Node::lowerOrder(size_t order) -> std::optional<Path>
{
    const auto preferedQuantity = [](size_t a, size_t b) -> bool {
        const std::array quantity = {4, 3, 5, 2, 6};
        const auto* a_it = ranges::find(quantity, a);
        const auto* b_it = ranges::find(quantity, b);
        if (a_it != b_it) {
            return a_it < b_it;
        }
        return a < b;
    };
    auto& cards = walkableData->Cards();
    const auto& thisTnv = cards[cardIndex].getTimingAndVocables();
    ranges::copy(subCards | views::filter([&cards, &thisTnv, order](size_t index) -> bool {
                     const auto& tnv = cards[index].getTimingAndVocables();
                     return tnv.timing <= 0 && tnv.vocables.size() < thisTnv.vocables.size();
                     // return walkableData->Cards()[index].VocableIndices().size() < order;
                 }),
                 std::back_inserter(cardsLessVocables));
    if (cardsLessVocables.empty()) {
        return {};
    }
    ranges::sort(cardsLessVocables, [&, this](size_t index_a, size_t index_b) -> bool {
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
    spdlog::info("lowerOrder subCards size: {}", cardsLessVocables.size());
    spdlog::info("smallCard size:{}, index: {}",
                 cards[cardsLessVocables[0]].getTimingAndVocables().vocables.size(),
                 cardsLessVocables[0]);
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
} // namespace sr
