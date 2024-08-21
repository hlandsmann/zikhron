#include "Tree.h"

#include "Node.h"

#include <DataBase.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <srtypes.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace sr {
Tree::Tree(std::shared_ptr<DataBase> _db,
           size_t _vocableIndex,
           CardId cardId,
           std::shared_ptr<cardId_set> _ignoreCardIds)
    : db{std::move(_db)}
    , nodes{std::make_shared<node_vector>()}
    , vocableIndex{_vocableIndex}
    , rootCardId{cardId}
    , ignoreCardIds{std::move(_ignoreCardIds)}
{
    (*nodes)[rootCardId].emplace(db, nodes, rootCardId, ignoreCardIds);
}

void Tree::build()
{
    auto& optionalRoot = (*nodes)[rootCardId];
    if (!optionalRoot.has_value()) {
        return;
    }
    auto& root = optionalRoot.value();
    auto order = root.lowerOrder(0);
    spdlog::info("Order for tree with card_index {} is {}", rootCardId, order);
}

auto Tree::getRoot() const -> CardId
{
    return rootCardId;
}

auto Tree::getNodeCardId() -> std::optional<CardId>
{
    std::optional<CardId> result = std::nullopt;
    spdlog::info("--- getNodeCardId ---");
    CardId cardId = rootCardId;
    spdlog::info("rootcardId: {}", rootCardId);
    auto* optionalNode = &(*nodes)[cardId];
    if (not optionalNode->has_value()) {
        spdlog::info("no value");
        return {};
    }
    if (optionalNode->value().Paths().empty()) {
        spdlog::info("empty");
        return {};
    }
    while (optionalNode->has_value() && (not optionalNode->value().Paths().empty())) {
        // Note: if(not optionalNode.has_value()) test avoids clang-tidy warning, but is superfluous
        // spdlog::info("while");
        if (not optionalNode->has_value()) {
            break;
        }
        auto& node = optionalNode->value();
        cardId = node.Paths().front().cardId;
        spdlog::info("cardId: {}", cardId);
        result.emplace(cardId);
        optionalNode = &(*nodes)[cardId];
    }

    return result;
}

} // namespace sr
