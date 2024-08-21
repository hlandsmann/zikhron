#pragma once
#include "Path.h"

#include <DataBase.h>
#include <misc/Identifier.h>
#include <srtypes.h>

#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace sr {

class Node;
using node_vector = std::map<CardId, std::optional<Node>>;

class Node
{
    static constexpr size_t s_stopBreakDown = 5;

public:
    Node() = default;
    Node(std::shared_ptr<DataBase> db,
         std::shared_ptr<node_vector> nodes,
         CardId cardId,
         std::shared_ptr<cardId_set> ignoreCardIds);

    void tighten();
    [[nodiscard]] auto lowerOrder(size_t order) -> size_t;
    [[nodiscard]] auto lowerOrderPulled() -> Path;
    [[nodiscard]] auto Paths() const -> const std::vector<Path>&;

    [[nodiscard]] auto CardID() const -> CardId { return nodeCardId; }

private:
    void traverseAndTighten(Path& path);
    [[nodiscard]] auto collectSubCards() const -> cardId_set;
    [[nodiscard]] auto removeInactiveCardIds(const cardId_set& cardIds) -> std::vector<CardId>;
    void sortCardIds(std::vector<CardId>& cardIds);
    std::shared_ptr<DataBase> db;
    std::weak_ptr<node_vector> weakNodes;
    CardId nodeCardId{};
    std::shared_ptr<cardId_set> ignoreCardIds;
    cardId_set subCards; // all cards that contain vocables that are contained by this
    std::vector<CardId> cardsLessVocables;
    std::vector<CardId> cardsLessVocablesPulled;
    std::vector<CardId> cardsMoreVocables;

    std::vector<Path> paths;
};
} // namespace sr
