#include <TreeWalker.h>
#include <WalkableData.h>
#include <bits/ranges_algo.h>
#include <spdlog/spdlog.h>
#include <utils/counting_iterator.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
// #include <ranges>
#include <ranges>
#include <utility>

#include <sys/types.h>

namespace ranges = std::ranges;
namespace views = std::ranges::views;

namespace {
void walk(const std::shared_ptr<sr::WalkableData>& walkableData)
{
    const auto& cards = walkableData->Cards();
    const auto& vocables = walkableData->Vocables();
    sr::index_set todayVocablesIndices;

    auto todayVocables = todayVocablesIndices
                         | views::transform([&vocables](size_t index) -> const sr::VocableMeta& {
                               return vocables[index];
                           });
    int cardIndex = 0;
    for (const auto& card : cards) {
        // if (cardIndex++ == 1200) {
        auto tnv = walkableData->timingAndNVocables(card);
        if (tnv.timing <= 0) {
            todayVocablesIndices.insert(tnv.vocables.begin(), tnv.vocables.end());
        }
        spdlog::info("Card: {}, when{}, activeVocs:{}", cardIndex++, tnv.timing, tnv.vocables.size());
        // }
    }
    spdlog::info("Vocables to study: {}", todayVocablesIndices.size());

    auto recency = [&vocables](size_t index) -> float { return vocables[index].Progress().recency(); };

    size_t vocable = *ranges::min_element(todayVocablesIndices, std::less{}, recency);
    spdlog::info("voc_index: {}, recency: {}", vocable, vocables[vocable].Progress().recency());
    for (size_t cIndex : vocables[vocable].CardIndices()) {
        auto tnv = walkableData->timingAndNVocables(cards[cIndex]);
        spdlog::info("+ cindex: {}, size: {}, time: {}", cIndex, tnv.vocables.size(), tnv.timing);
        for (size_t vIndex : tnv.vocables) {
            spdlog::info("| + vindex: {}, size: {}, recency: {}",
                         vIndex, vocables[vIndex].CardIndices().size(), vocables[vIndex].Progress().recency());
        }
        spdlog::info("...");
        for (size_t vIndex : cards[cIndex].VocableIndices()) {
            spdlog::info("| + vindex: {}, size: {}, recency: {}",
                         vIndex, vocables[vIndex].CardIndices().size(), vocables[vIndex].Progress().recency());
        }
    }
}

} // namespace

namespace sr {
Node::Node(std::shared_ptr<WalkableData> _walkableData)
    : walkableData{std::move(_walkableData)} {}

// void Node::push(uint cardIndex)
// {
//     const auto& card = walkableData->Cards()[cardIndex];
//     const auto& tnv = walkableData->timingAndNVocables(card);
//     if (tnv.timing > 0) {
//         return;
//     }
//     // index_set intersection;
//     // ranges::set_intersection(tnv.vocables, vocables, std::inserter(intersection, intersection.begin()));
//     size_t intersectionCount = ranges::set_intersection(
//                                        tnv.vocables, vocables, utl::counting_iterator{})
//                                        .out.count;
//     if (intersectionCount == 0) {
//     }
// }

TreeWalker::TreeWalker(std::shared_ptr<WalkableData> _walkableData)
    : walkableData{std::move(_walkableData)}
    , root{walkableData}
{
    walk(walkableData);
}
} // namespace sr
