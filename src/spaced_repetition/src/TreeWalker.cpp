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
#include <optional>
#include <vector>
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
        // spdlog::info("Card: {}, when{}, activeVocs:{}", cardIndex++, tnv.timing, tnv.vocables.size());
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
Node::Node(std::shared_ptr<WalkableData> _walkableData,
           std::vector<std::optional<std::shared_ptr<Node>>>& nodes,
           size_t _cardIndex)
    : walkableData{std::move(_walkableData)}
    , cardIndex{_cardIndex}
    , subCards{collectSubCards()}
{
    spdlog::info("Subcards size {}", subCards.size());
}

auto Node::collectSubCards() const -> index_set
{
    index_set subCardsResult;

    const auto& card = walkableData->Cards()[cardIndex];
    const auto& tnv = walkableData->timingAndNVocables(card);
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

Tree::Tree(std::shared_ptr<WalkableData> _walkableData, size_t _vocableIndex, size_t _cardIndex)
    : walkableData{std::move(_walkableData)}
    , vocableIndex{_vocableIndex}
    , cardIndex{_cardIndex}
{
    nodes.resize(walkableData->Cards().size());
    root = std::make_shared<Node>(walkableData, nodes, cardIndex);
    nodes[cardIndex] = root;
}

TreeWalker::TreeWalker(std::shared_ptr<WalkableData> _walkableData)
    : walkableData{std::move(_walkableData)}
{
    walk(walkableData);
    createTree();
}

auto TreeWalker::getTodayVocables() const -> index_set
{
    const auto& cards = walkableData->Cards();
    sr::index_set todayVocables;
    for (const auto& card : cards) {
        auto tnv = walkableData->timingAndNVocables(card);
        if (tnv.timing <= 0) {
            todayVocables.insert(tnv.vocables.begin(), tnv.vocables.end());
        }
    }
    return todayVocables;
}

auto TreeWalker::getNextTargetVocable() const -> std::optional<size_t>
{
    const auto& vocables = walkableData->Vocables();
    sr::index_set todayVocables = getTodayVocables();
    if (todayVocables.empty()) {
        return {};
    }
    auto recency = [&vocables](size_t index) -> float { return vocables[index].Progress().recency(); };
    size_t targetVocable = *ranges::min_element(todayVocables, std::less{}, recency);

    return {targetVocable};
}

auto TreeWalker::getNextTargetCard(size_t vocableIndex) const -> size_t
{
    const auto& vocables = walkableData->Vocables();
    return *vocables[vocableIndex].CardIndices().begin();
}

void TreeWalker::createTree()
{
    auto optionalTargetVocable = getNextTargetVocable();
    if (!optionalTargetVocable.has_value()) {
        return;
    }
    auto cardIndex = getNextTargetCard(optionalTargetVocable.value());
    tree = std::make_unique<Tree>(walkableData, optionalTargetVocable.value(), cardIndex);
}

} // namespace sr
