#pragma once
#include "CardMeta.h"
#include "DataBase.h"
#include "ITreeWalker.h"
#include "Tree.h"
#include "srtypes.h"

#include <annotation/Ease.h>
#include <database/CbdFwd.h>
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
    // void setEaseForCard(database::CardPtr cardId, const Id_Ease_vt& id_ease) override;
    auto getNextCardChoice() -> const CardMeta& override;

    [[nodiscard]] auto getNumberOfFailedVocables() const -> unsigned override;
    [[nodiscard]] auto getNumberOfTodayVocables() const -> unsigned override;

private:
    void updateFailedVocables();
    [[nodiscard]] auto getTodayVocables() const -> index_set;
    [[nodiscard]] auto getNextTargetVocable(const std::shared_ptr<cardId_set>& ignoreCards) const -> std::optional<size_t>;
    [[nodiscard]] auto getNextTargetCard() -> std::optional<CardId>;
    [[nodiscard]] auto getFailedVocableCleanTreeCardId(
            const std::shared_ptr<cardId_set>& ignoreCardIds) -> std::optional<CardId>;
    [[nodiscard]] auto getFailedVocIgnoreCardIds() const -> std::shared_ptr<cardId_set>;
    auto removeRepeatNowCardId(const std::shared_ptr<cardId_set>& ignoreCards) -> bool;
    void addNextVocableToIgnoreCardIds(size_t nextVocable, std::shared_ptr<cardId_set>&);

    [[nodiscard]] auto createTree(size_t targetVocableIndex, std::shared_ptr<cardId_set>) const -> Tree;

    std::shared_ptr<DataBase> db;
    // std::optional<Tree> tree;
    std::map<size_t, std::optional<Tree>> vocableIndex_tree;
    index_set failedVocables;
    unsigned numberOfFailedVocables{};

    mutable unsigned numberOfTodayVocables{};
};

} // namespace sr
