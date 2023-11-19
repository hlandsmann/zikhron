#pragma once
#include "CardMeta.h"
#include "DataBase.h"
#include "ITreeWalker.h"

#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/ManualExecutor.h>
#include <folly/experimental/coro/Task.h>
#include <folly/futures/Future.h>
#include <folly/futures/SharedPromise.h>
#include <misc/Config.h>

#include <memory>

namespace sr {
class AsyncTreeWalker
{
public:
    using DataBasePtr = std::shared_ptr<DataBase>;
    using TreeWalkerPtr = std::shared_ptr<ITreeWalker>;
    AsyncTreeWalker(std::shared_ptr<folly::ManualExecutor> synchronousExecutor,
                    std::shared_ptr<folly::CPUThreadPoolExecutor> threadPoolExecutor);
    auto getDataBase() const -> folly::Future<DataBasePtr>;
    auto getTreeWalker() const -> folly::Future<TreeWalkerPtr>;
    auto getNextCardChoice() -> folly::coro::Task<CardMeta>;

private:
    static auto get_zikhron_cfg() -> std::shared_ptr<zikhron::Config>;
    static auto taskCreateDataBase() -> folly::coro::Task<DataBasePtr>;
    auto taskFullfillPromises() -> folly::coro::Task<>;

    folly::SharedPromise<DataBasePtr> dataBasePromise;
    folly::SharedPromise<TreeWalkerPtr> treeWalkerPromise;
    std::shared_ptr<folly::CPUThreadPoolExecutor> threadPoolExecutor;
    std::shared_ptr<folly::ManualExecutor> synchronousExecutor;
};
} // namespace sr
