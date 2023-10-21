#pragma once
#include "ITreeWalker.h"
#include "Tree.h"

#include <annotation/CardDB.h>
#include <annotation/Ease.h>
#include <annotation/ZH_Annotator.h>
#include <misc/Identifier.h>
#include <utils/StringU8.h>

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
    TreeWalker(std::shared_ptr<DataBase>);
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
    // auto AddVocableChoice(VocableId vocId, VocableId vocIdOldChoice, VocableId vocIdNewChoice)
    //         -> CardInformation override;
    // auto AddAnnotation(const ZH_Annotator::Combination& combination,
    //                    const std::vector<utl::CharU8>& characterSequence)
    //         -> CardInformation override;

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

    std::shared_ptr<DataBase> walkableData;
    // std::optional<Tree> tree;
    std::map<size_t, std::optional<Tree>> vocableIndex_tree;
    index_set failedVocables;

    size_t currentCardIndex{};
};

} // namespace sr
