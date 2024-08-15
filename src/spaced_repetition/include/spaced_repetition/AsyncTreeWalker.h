#pragma once
#include "CardMeta.h"
#include "DataBase.h"
#include "ITreeWalker.h"

#include <misc/Config.h>
#include <misc/Identifier.h>

#include <kocoro/kocoro.hpp>
#include <memory>
#include <optional>

namespace sr {
class AsyncTreeWalker
{
public:
    using DataBasePtr = std::shared_ptr<DataBase>;
    using TreeWalkerPtr = std::shared_ptr<ITreeWalker>;
    AsyncTreeWalker(std::shared_ptr<kocoro::SynchronousExecutor> synchronousExecutor);

    [[nodiscard]] auto getDataBase() const -> kocoro::Async<DataBasePtr>&;
    [[nodiscard]] auto getTreeWalker() const -> kocoro::Async<TreeWalkerPtr>&;
    auto getNextCardChoice() -> kocoro::Async<CardMeta>&;

private:
    static auto taskCreateDataBase() -> kocoro::Task<DataBasePtr>;
    auto taskFullfillPromises() -> kocoro::Task<>;

    kocoro::AsyncPtr<DataBasePtr> asyncDataBase;
    kocoro::AsyncPtr<TreeWalkerPtr> asyncTreewalker;
    kocoro::AsyncPtr<CardMeta> asyncNextCard;

    TreeWalkerPtr treeWalker;
};
} // namespace sr
