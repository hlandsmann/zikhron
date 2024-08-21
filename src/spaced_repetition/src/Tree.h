#pragma once
#include "Node.h"
#include "srtypes.h"

#include <DataBase.h>
#include <misc/Identifier.h>

#include <cstddef>
#include <memory>
#include <optional>

namespace sr {
class Tree
{
public:
    Tree(std::shared_ptr<DataBase> db,
         size_t vocableIndex,
         CardId cardId,
         std::shared_ptr<cardId_set> ignoreCardIds);
    void build();
    [[nodiscard]] auto getRoot() const -> CardId;
    [[nodiscard]] auto getNodeCardId() -> std::optional<CardId>;

private:
    std::shared_ptr<DataBase> db;
    std::shared_ptr<node_vector> nodes;
    size_t vocableIndex;
    CardId rootCardId;
    std::shared_ptr<cardId_set> ignoreCardIds;
};
} // namespace sr
