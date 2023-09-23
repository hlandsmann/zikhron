#pragma once
#include "WalkableData.h"

#include <memory>
#include <optional>
#include <vector>

#include <sys/types.h>

namespace sr {
class Node
{
public:
    Node(std::shared_ptr<WalkableData> walkableData);
    void push(uint cardIndex);

private:
    void pushDown(uint cardIndex);

    Node* parent{nullptr};
    index_set vocables;
    std::vector<Node> nodes;

    std::optional<uint> cardIndex;
    bool active{false};
    std::shared_ptr<WalkableData> walkableData;
};

class TreeWalker
{
public:
    TreeWalker(std::shared_ptr<WalkableData>);

private:
    std::shared_ptr<WalkableData> walkableData;
    Node root;
};
} // namespace sr
