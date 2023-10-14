#pragma once
#include "ITreeWalker.h"
#include "Tree.h"
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

class Tree;

class TreeWalker : public ITreeWalker
{
public:
    TreeWalker(std::shared_ptr<WalkableData>);
    TreeWalker(const TreeWalker&) = delete;
    TreeWalker(TreeWalker&&) = delete;
    auto operator=(const TreeWalker& other) -> TreeWalker& = delete;
    auto operator=(TreeWalker&& other) noexcept -> TreeWalker& = delete;
    ~TreeWalker() override = default;

    using VocableIds_vt = std::vector<VocableId>;
    using Id_Ease_vt = std::map<VocableId, Ease>;
    using CardInformation = std::tuple<std::optional<std::unique_ptr<Card>>, VocableIds_vt, Id_Ease_vt>;
    auto getNextCardChoice(std::optional<CardId> preferedCardId = {}) -> CardInformation override;
    void setEaseLastCard(const Id_Ease_vt& id_ease) override;
    void saveProgress() const override;

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
