#pragma once
#include "Path.h"

#include <DataBase.h>
#include <misc/Identifier.h>
#include <srtypes.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace sr {

class Node;
using node_vector = std::vector<std::optional<Node>>;

class Node
{
    static constexpr size_t s_stopBreakDown = 5;

public:
    Node() = default;
    Node(std::shared_ptr<DataBase> db,
         std::shared_ptr<node_vector> nodes,
         size_t cardIndex,
         std::shared_ptr<index_set> ignoreCardIndices);

    void tighten();
    [[nodiscard]] auto lowerOrder(size_t order) -> size_t;
    [[nodiscard]] auto lowerOrderPulled() -> Path;
    [[nodiscard]] auto Paths() const -> const std::vector<Path>&;

    [[nodiscard]] auto CardID() const -> CardId { return db->Cards().id_from_index(cardIndex); }

private:
    void traverseAndTighten(Path& path);
    [[nodiscard]] auto collectSubCards() const -> index_set;
    [[nodiscard]] auto removeInactiveCardindices(const index_set& cardIndices) -> std::vector<size_t>;
    void sortCardIndices(std::vector<size_t>& cardIndices);
    std::shared_ptr<DataBase> db;
    std::weak_ptr<node_vector> weakNodes;
    size_t cardIndex{};
    std::shared_ptr<index_set> ignoreCardIndices;
    index_set subCards; // all cards that contain vocables that are contained by this
    std::vector<size_t> cardsLessVocables;
    std::vector<size_t> cardsLessVocablesPulled;
    std::vector<size_t> cardsMoreVocables;

    std::vector<Path> paths;
};
} // namespace sr
