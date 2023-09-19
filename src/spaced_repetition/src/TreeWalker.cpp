#include <TreeWalker.h>
#include <bits/ranges_algo.h>
#include <spdlog/spdlog.h>
#include <utils/counting_iterator.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <ranges>

namespace ranges = std::ranges;
namespace views = std::ranges::views;

namespace {
void walk(const std::shared_ptr<sr::WalkableData>& walkableData)
{
    const auto& cards = walkableData->Cards();
    const auto& vocables = walkableData->Vocables();
    sr::index_set todayVocables;
    int cardIndex = 0;
    for (const auto& card : cards) {
        // if (cardIndex++ == 1200) {
        auto tnv = walkableData->timingAndNVocables(card);
        if (tnv.timing <= 0) {
            todayVocables.insert(tnv.vocables.begin(), tnv.vocables.end());
        }
        spdlog::info("Card: {}, when{}, activeVocs:{}", cardIndex++, tnv.timing, tnv.vocables.size());
        // }
    }

    spdlog::info("Vocables to study: {}", todayVocables.size());
}

} // namespace

namespace sr {
Node::Node(std::shared_ptr<WalkableData> _walkableData)
    : walkableData{std::move(_walkableData)} {}

void Node::push(uint cardIndex)
{
    const auto& card = walkableData->Cards()[cardIndex];
    const auto& tnv = walkableData->timingAndNVocables(card);
    if (tnv.timing > 0) {
        return;
    }
    // index_set intersection;
    // ranges::set_intersection(tnv.vocables, vocables, std::inserter(intersection, intersection.begin()));
    size_t intersectionCount = ranges::set_intersection(
                                       tnv.vocables, vocables, utl::counting_iterator{})
                                       .out.count;
    if (intersectionCount == 0) {
    }
}

TreeWalker::TreeWalker(std::shared_ptr<WalkableData> _walkableData)
    : walkableData{std::move(_walkableData)}
    , root{walkableData}
{
    walk(walkableData);
}
} // namespace sr
