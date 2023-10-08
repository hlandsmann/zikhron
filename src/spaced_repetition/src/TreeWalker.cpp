#include <TreeWalker.h>
#include <WalkableData.h>
#include <bits/ranges_algo.h>
#include <spdlog/spdlog.h>
#include <utils/counting_iterator.h>

#include <algorithm>
#include <array>
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
    // const auto& cards = walkableData->Cards();
    // const auto& vocables = walkableData->Vocables();
    // sr::index_set todayVocablesIndices;
    //
    // auto todayVocables = todayVocablesIndices
    //                      | views::transform([&vocables](size_t index) -> const sr::VocableMeta& {
    //                            return vocables[index];
    //                        });
    // int cardIndex = 0;
    // for (const auto& card : cards) {
    //     // if (cardIndex++ == 1200) {
    //     auto tnv = walkableData->timingAndNVocables(card);
    //     if (tnv.timing <= 0) {
    //         todayVocablesIndices.insert(tnv.vocables.begin(), tnv.vocables.end());
    //     }
    //     // spdlog::info("Card: {}, when{}, activeVocs:{}", cardIndex++, tnv.timing, tnv.vocables.size());
    //     // }
    // }
    // spdlog::info("Vocables to study: {}", todayVocablesIndices.size());
    //
    // auto recency = [&vocables](size_t index) -> float { return vocables[index].Progress().recency(); };
    //
    // size_t vocable = *ranges::min_element(todayVocablesIndices, std::less{}, recency);
    // spdlog::info("voc_index: {}, recency: {}", vocable, vocables[vocable].Progress().recency());
    // for (size_t cIndex : vocables[vocable].CardIndices()) {
    //     auto tnv = walkableData->timingAndNVocables(cards[cIndex]);
    //     spdlog::info("+ cindex: {}, size: {}, time: {}", cIndex, tnv.vocables.size(), tnv.timing);
    //     for (size_t vIndex : tnv.vocables) {
    //         spdlog::info("| + vindex: {}, size: {}, recency: {}",
    //                      vIndex, vocables[vIndex].CardIndices().size(), vocables[vIndex].Progress().recency());
    //     }
    //     spdlog::info("...");
    //     for (size_t vIndex : cards[cIndex].VocableIndices()) {
    //         spdlog::info("| + vindex: {}, size: {}, recency: {}",
    //                      vIndex, vocables[vIndex].CardIndices().size(), vocables[vIndex].Progress().recency());
    //     }
    // }
}

} // namespace

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
    , nodes{std::make_shared<node_vector>()}
    , vocableIndex{_vocableIndex}
    , cardIndex{_cardIndex}
{
    nodes->resize(walkableData->Cards().size());
    (*nodes)[cardIndex].emplace(walkableData, nodes, cardIndex);
}

void Tree::build()
{
    auto optionalRoot = (*nodes)[cardIndex];
    if (!optionalRoot.has_value()) {
        return;
    }
    auto& root = optionalRoot.value();
    auto optionalPath = root.lowerOrder(0);
    if (optionalPath.has_value()) {
        paths.push_back(*optionalPath);
    }
}

auto Tree::Paths() const -> const std::vector<Path>&
{
    return paths;
}

auto Tree::getRoot() const -> size_t
{
    return cardIndex;
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

auto TreeWalker::getNextCardChoice(std::optional<uint> preferedCardId) -> CardInformation
{
    createTree();
    const auto& paths = tree->Paths();
    size_t activeCardIndex{};
    if (!paths.empty()) {
        activeCardIndex = tree->Paths().front().cardIndex;
    } else {
        activeCardIndex = tree->getRoot();
    }
    auto card = walkableData->getCardCopy(activeCardIndex);

    return {std::move(card),
            walkableData->getVocableIdsInOrder(activeCardIndex),
            walkableData->getRelevantEase(activeCardIndex)};
}

void TreeWalker::setEaseLastCard(const Id_Ease_vt& id_ease)
{
    for (auto [vocId, ease] : id_ease) {
        walkableData->setEaseVocable(vocId, ease);
        walkableData->resetCardsContainingVocable(vocId);
    }
}

void TreeWalker::createTree()
{
    auto optionalTargetVocable = getNextTargetVocable();
    if (!optionalTargetVocable.has_value()) {
        return;
    }
    auto cardIndex = getNextTargetCard(optionalTargetVocable.value());

    spdlog::info("TargetVocable: {}, cardSize: {}", optionalTargetVocable.value(),
                 walkableData->Cards()[cardIndex].getTimingAndVocables().vocables.size());
    tree = std::make_unique<Tree>(walkableData, optionalTargetVocable.value(), cardIndex);
    tree->build();
}

void TreeWalker::saveProgress() const
{
    walkableData->saveProgress();
}
} // namespace sr
