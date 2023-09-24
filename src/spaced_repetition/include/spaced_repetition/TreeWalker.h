#pragma once
#include "WalkableData.h"

#include <memory>
#include <optional>
#include <vector>

#include <sys/types.h>

namespace sr {

class Path
{
public:
private:
    uint steps{};
    uint eliminateCount{};
    uint cardIndex;
};

class Node
{
public:
    Node(std::shared_ptr<WalkableData> walkableData);

private:
    std::vector<uint> cardsLessVocables;
    std::vector<uint> cardsLessVocablesPulled;
    std::vector<uint> cardsMoreVocables;

    std::shared_ptr<WalkableData> walkableData;
    std::vector<Path> paths;
};

class Tree
{
public:
private:
    std::vector<std::optional<std::shared_ptr<Node>>> nodes;
    std::shared_ptr<Node> root;
    uint vocableIndex;
    uint cardIndex;
};

class TreeWalker
{
public:
    TreeWalker(std::shared_ptr<WalkableData>);

private:
    void createTree();


    std::shared_ptr<WalkableData> walkableData;
    std::shared_ptr<Tree> tree;
};

} // namespace sr
