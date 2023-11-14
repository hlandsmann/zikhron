#include <AsyncDataBase.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/ManualExecutor.h>
#include <folly/experimental/coro/Task.h>
#include <folly/futures/Future.h>
#include <misc/Config.h>
#include <spaced_repetition/DataBase.h>
#include <spaced_repetition/ITreeWalker.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <memory>
#include <utility>

namespace fs = std::filesystem;

AsyncDataBase::AsyncDataBase(std::shared_ptr<folly::CPUThreadPoolExecutor> _threadPoolExecutor,
                             std::shared_ptr<folly::ManualExecutor> _synchronousExecutor)
    : threadPoolExecutor{std::move(_threadPoolExecutor)}
    , synchronousExecutor{std::move(_synchronousExecutor)}
{
    taskFullfillPromises().semi().via(synchronousExecutor.get());
}

auto AsyncDataBase::getDataBase() -> folly::Future<DataBasePtr>
{
    return dataBasePromise.getFuture();
}

auto AsyncDataBase::getTreeWalker() -> folly::Future<TreeWalkerPtr>
{
    return treeWalkerPromise.getFuture();
}

auto AsyncDataBase::get_zikhron_cfg() -> std::shared_ptr<zikhron::Config>
{
    auto path_to_exe = fs::read_symlink("/proc/self/exe");
    auto zikhron_cfg_json = path_to_exe.parent_path() / "config.json";
    return std::make_shared<zikhron::Config>(path_to_exe.parent_path());
}

auto AsyncDataBase::taskFullfillPromises() -> folly::coro::Task<>
{
    auto dbPtr = co_await taskCreateDataBase().scheduleOn(threadPoolExecutor.get());
    dataBasePromise.setValue(dbPtr);
    auto treeWalkerPtr = sr::ITreeWalker::createTreeWalker(std::move(dbPtr));
    treeWalkerPromise.setValue(std::move(treeWalkerPtr));
    co_return;
}

auto AsyncDataBase::taskCreateDataBase() -> folly::coro::Task<DataBasePtr>
{
    auto zikhron_cfg = get_zikhron_cfg();
    auto db = std::make_unique<sr::DataBase>(zikhron_cfg);
    co_return db;
}
