#pragma once
#include "Node.h"

#include <DataBase.h>

#include <cstddef>
#include <memory>
#include <optional>
namespace sr {
class Tree
{
public:
    Tree(std::shared_ptr<DataBase> walkableData,
         size_t vocableIndex,
         size_t cardIndex,
         std::shared_ptr<index_set> ignoreCardIndices);
    void build();
    [[nodiscard]] auto getRoot() const -> size_t;
    [[nodiscard]] auto getNodeCardIndex() -> std::optional<size_t>;

private:
    std::shared_ptr<DataBase> walkableData;
    std::shared_ptr<node_vector> nodes;
    size_t vocableIndex;
    size_t rootCardIndex;
    std::shared_ptr<index_set> ignoreCardIndices;
};
} // namespace sr
