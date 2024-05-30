#pragma once
#include "CardMeta.h"
#include "DataBase.h"
#include "ITreeWalker.h"
#include "Tree.h"
#include "srtypes.h"

#include <annotation/CardDB.h>
#include <annotation/Ease.h>
#include <misc/Identifier.h>
#include <utils/StringU8.h>

#include <cstddef>
#include <map>
#include <memory>
#include <optional>

#include <sys/types.h>

namespace sr {

class Tree;

class TreeWalker : public ITreeWalker
{
public:
    TreeWalker(std::shared_ptr<DataBase>);
    TreeWalker(const TreeWalker&) = delete;
    TreeWalker(TreeWalker&&) = delete;
    auto operator=(const TreeWalker& other) -> TreeWalker& = delete;
    auto operator=(TreeWalker&& other) noexcept -> TreeWalker& = delete;
    ~TreeWalker() override = default;

    using Id_Ease_vt = ITreeWalker::Id_Ease_vt;
    void setEaseLastCard(const Id_Ease_vt& id_ease) override;
    auto getNextCardChoice(std::optional<CardId> preferedCardId = {}) -> CardMeta& override;
    auto getLastCard() -> CardMeta& override;

private:
    [[nodiscard]] auto getTodayVocables() const -> index_set;
    [[nodiscard]] auto getNextTargetVocable(const std::shared_ptr<index_set>& ignoreCards) const -> std::optional<size_t>;
    [[nodiscard]] auto getNextTargetCard() -> std::optional<size_t>;
    [[nodiscard]] auto getFailedVocableCleanTreeCardIndex(
            const std::shared_ptr<index_set>& ignoreCardIndices) -> std::optional<size_t>;
    [[nodiscard]] auto getFailedVocIgnoreCardIndices() const -> std::shared_ptr<index_set>;
    auto removeRepeatNowCardIndex(const std::shared_ptr<index_set>& ignoreCards) -> bool;
    void addNextVocableToIgnoreCardIndices(size_t nextVocable, std::shared_ptr<index_set>&);

    [[nodiscard]] auto createTree(size_t targetVocableIndex, std::shared_ptr<index_set>) const -> Tree;

    std::shared_ptr<DataBase> db;
    // std::optional<Tree> tree;
    std::map<size_t, std::optional<Tree>> vocableIndex_tree;
    index_set failedVocables;

    size_t currentCardIndex{};
};

} // namespace sr
