#pragma once
#include <cstddef>
#include <WalkableData.h>
#include <vector>
#include <optional>
#include <memory>
 
namespace sr {
struct Path
{
    size_t cardIndex;
    // uint steps{};
    // uint eliminateCount{};
};

class Node;
using node_vector = std::vector<std::optional<Node>>;
class Node
{
    static constexpr size_t s_stopBreakDown = 3;

public:
    Node(std::shared_ptr<WalkableData> walkableData,
         std::shared_ptr<node_vector> nodes,
         size_t cardIndex);
    [[nodiscard]] auto lowerOrder(size_t order) -> std::optional<Path>;
    [[nodiscard]] auto lowerOrderPulled() -> Path;

private:
    [[nodiscard]] auto collectSubCards() const -> index_set;
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
