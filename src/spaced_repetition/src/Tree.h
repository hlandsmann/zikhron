#pragma once
#include <optional>
#include <cstddef>
#include "Node.h"
#include <vector>
#include <WalkableData.h>

#include <memory>
namespace sr {
class Tree
{
public:
    Tree(std::shared_ptr<WalkableData> walkableData, size_t vocableIndex, size_t cardIndex);
    void build();
    [[nodiscard]] auto getRoot() const -> size_t;
    [[nodiscard]] auto getNodeCardIndex() -> std::optional<size_t>;

private:
    std::shared_ptr<WalkableData> walkableData;
    std::shared_ptr<node_vector> nodes;
    size_t vocableIndex;
    size_t rootCardIndex;
};
} // namespace sr
