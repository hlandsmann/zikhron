#include "Tree.h"

#include "Node.h"

#include <DataBase.h>
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
           size_t cardIndex,
           std::shared_ptr<index_set> _ignoreCardIndices)
    : db{std::move(_db)}
    , nodes{std::make_shared<node_vector>()}
    , vocableIndex{_vocableIndex}
    , rootCardIndex{cardIndex}
    , ignoreCardIndices{std::move(_ignoreCardIndices)}
{
    nodes->resize(db->Cards().size());
    (*nodes)[rootCardIndex].emplace(db, nodes, rootCardIndex, ignoreCardIndices);
}

void Tree::build()
{
    auto& optionalRoot = (*nodes)[rootCardIndex];
    if (!optionalRoot.has_value()) {
        return;
    }
    auto& root = optionalRoot.value();
    auto order = root.lowerOrder(0);
    spdlog::info("Order for tree with card_index {} is {}", rootCardIndex, order);
}

auto Tree::getRoot() const -> size_t
{
    return rootCardIndex;
}

auto Tree::getNodeCardIndex() -> std::optional<size_t>
{
    std::optional<size_t> result = std::nullopt;
    spdlog::info("--- getNodeCardIndex ---");
    size_t cardIndex = rootCardIndex;
    spdlog::info("rootcardId: {}", db->Cards().id_from_index(cardIndex));
    auto* optionalNode = &(*nodes)[cardIndex];
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
        cardIndex = node.Paths().front().cardIndex;
        spdlog::info("cardId: {}", db->Cards().id_from_index(cardIndex));
        result.emplace(cardIndex);
        optionalNode = &(*nodes)[cardIndex];
    }

    return result;
}

} // namespace sr
