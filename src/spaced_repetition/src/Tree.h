#pragma once
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
    [[nodiscard]] auto Paths() const -> const std::vector<Path>&;
    [[nodiscard]] auto getRoot() const -> size_t;

private:
    std::shared_ptr<WalkableData> walkableData;
    std::shared_ptr<node_vector> nodes;
    std::vector<Path> paths;
    size_t vocableIndex;
    size_t cardIndex;
};
} // namespace sr
