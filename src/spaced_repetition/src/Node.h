#pragma once
#include <WalkableData.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace sr {
struct Path
{
    size_t cardIndex;
    size_t maxVocableSize;
    size_t steps;
    size_t eliminateCount;
};

class Node;
using node_vector = std::vector<std::optional<Node>>;
class Node
{
    static constexpr size_t s_stopBreakDown = 4;

public:
    Node(std::shared_ptr<WalkableData> walkableData,
         std::shared_ptr<node_vector> nodes,
         size_t cardIndex);
    [[nodiscard]] auto lowerOrder(size_t order) -> std::optional<Path>;
    [[nodiscard]] auto lowerOrderPulled() -> Path;

private:
    [[nodiscard]] auto collectSubCards() const -> index_set;
    [[nodiscard]] auto removeInactiveCardindices(const index_set& cardIndices) -> std::vector<size_t>;
    void sortCardIndices(std::vector<size_t>& cardIndices);
    std::shared_ptr<WalkableData> walkableData;
    std::shared_ptr<node_vector> nodes;
    size_t cardIndex;
    index_set subCards; // all cards that contain vocables that are contained by this
    std::vector<size_t> cardsLessVocables;
    std::vector<size_t> cardsLessVocablesPulled;
    std::vector<size_t> cardsMoreVocables;

    std::vector<Path> paths;
};
} // namespace sr
