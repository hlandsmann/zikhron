#include "Tree.h"
#include <cstddef>
#include <utility>
#include "Node.h"
#include <vector>

#include <WalkableData.h>

#include <memory>
namespace sr {
Tree::Tree(std::shared_ptr<WalkableData> _walkableData, size_t _vocableIndex, size_t _cardIndex)
    : walkableData{std::move(_walkableData)}
    , nodes{std::make_shared<node_vector>()}
    , vocableIndex{_vocableIndex}
    , cardIndex{_cardIndex}
{
    nodes->resize(walkableData->Cards().size());
    (*nodes)[cardIndex].emplace(walkableData, nodes, cardIndex);
}

void Tree::build()
{
    auto optionalRoot = (*nodes)[cardIndex];
    if (!optionalRoot.has_value()) {
        return;
    }
    auto& root = optionalRoot.value();
    auto optionalPath = root.lowerOrder(0);
    if (optionalPath.has_value()) {
        paths.push_back(*optionalPath);
    }
}

auto Tree::Paths() const -> const std::vector<Path>&
{
    return paths;
}

auto Tree::getRoot() const -> size_t
{
    return cardIndex;
}
} // namespace sr
