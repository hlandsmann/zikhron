#pragma once
#include "WalkableData.h"

#include <annotation/Card.h>
#include <annotation/Ease.h>
#include <misc/Identifier.h>

#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <tuple>
#include <vector>

#include <sys/types.h>

namespace sr {

struct Path
{
    uint cardIndex;
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
    std::vector<uint> cardsLessVocables;
    std::vector<uint> cardsLessVocablesPulled;
    std::vector<uint> cardsMoreVocables;

    std::vector<Path> paths;
};

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

class TreeWalker
{
public:
    TreeWalker(std::shared_ptr<WalkableData>);
    using VocableIds_vt = std::vector<VocableId>;
    using Id_Ease_vt = std::map<VocableId, Ease>;
    using CardInformation = std::tuple<std::optional<std::unique_ptr<Card>>, VocableIds_vt, Id_Ease_vt>;
    auto getNextCardChoice(std::optional<CardId> preferedCardId = {}) -> CardInformation;
    void setEaseLastCard(const Id_Ease_vt& id_ease);
    void saveProgress() const;

private:
    [[nodiscard]] auto getTodayVocables() const -> index_set;
    [[nodiscard]] auto getNextTargetVocable() const -> std::optional<size_t>;
    [[nodiscard]] auto getNextTargetCard(size_t vocableIndex) const -> size_t;

    [[nodiscard]] auto createTree() -> std::optional<Tree>;

    std::shared_ptr<WalkableData> walkableData;
    std::optional<Tree> tree;
    index_set failedVocables;
};

} // namespace sr
