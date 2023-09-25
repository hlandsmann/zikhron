#pragma once
#include "WalkableData.h"

#include <cstddef>
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
    static constexpr size_t s_stopBreakDown = 3;

public:
    Node(std::shared_ptr<WalkableData> walkableData,
         std::vector<std::optional<std::shared_ptr<Node>>>& nodes,
         size_t cardIndex);

private:
    [[nodiscard]] auto collectSubCards() const -> index_set;
    std::shared_ptr<WalkableData> walkableData;
    size_t cardIndex;
    index_set subCards; // all cards that contain vocables that are contained by this
    std::vector<uint> cardsLessVocables;
    std::vector<uint> cardsLessVocablesPulled;
    std::vector<uint> cardsMoreVocables;

    std::vector<Path> paths;
};

class Tree
{
public:
    Tree(std::shared_ptr<WalkableData> walkableData, size_t vocableIndex, size_t cardIndex);

private:
    std::shared_ptr<WalkableData> walkableData;
    std::vector<std::optional<std::shared_ptr<Node>>> nodes;
    std::shared_ptr<Node> root;
    size_t vocableIndex;
    size_t cardIndex;
};

class TreeWalker
{
public:
    TreeWalker(std::shared_ptr<WalkableData>);

private:
    [[nodiscard]] auto getTodayVocables() const -> index_set;
    [[nodiscard]] auto getNextTargetVocable() const -> std::optional<size_t>;
    [[nodiscard]] auto getNextTargetCard(size_t vocableIndex) const -> size_t;

    void createTree();

    std::shared_ptr<WalkableData> walkableData;
    std::unique_ptr<Tree> tree;
};

} // namespace sr
