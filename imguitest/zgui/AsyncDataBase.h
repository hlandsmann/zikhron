#pragma once
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/ManualExecutor.h>
#include <folly/experimental/coro/Task.h>
#include <folly/futures/Future.h>
#include <folly/futures/SharedPromise.h>
#include <misc/Config.h>
#include <spaced_repetition/DataBase.h>
#include <spaced_repetition/ITreeWalker.h>

#include <memory>
class AsyncDataBase
{
public:
    using DataBasePtr = std::shared_ptr<sr::DataBase>;
    using TreeWalkerPtr = std::shared_ptr<sr::ITreeWalker>;
    AsyncDataBase(std::shared_ptr<folly::CPUThreadPoolExecutor> threadPoolExecutor,
                  std::shared_ptr<folly::ManualExecutor> synchronousExecutor);
    auto getDataBase() -> folly::Future<DataBasePtr>;
    auto getTreeWalker() -> folly::Future<TreeWalkerPtr>;

private:
    static auto get_zikhron_cfg() -> std::shared_ptr<zikhron::Config>;
    static auto taskCreateDataBase() -> folly::coro::Task<DataBasePtr>;
    auto taskFullfillPromises() -> folly::coro::Task<>;

    folly::SharedPromise<DataBasePtr> dataBasePromise;
    folly::SharedPromise<TreeWalkerPtr> treeWalkerPromise;
    std::shared_ptr<folly::CPUThreadPoolExecutor> threadPoolExecutor;
    std::shared_ptr<folly::ManualExecutor> synchronousExecutor;
};
