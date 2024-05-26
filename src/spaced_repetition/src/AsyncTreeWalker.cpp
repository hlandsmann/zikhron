#include <AsyncTreeWalker.h>
#include <CardMeta.h>
#include <DataBase.h>
#include <ITreeWalker.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <kocoro/kocoro.hpp>
#include <memory>
#include <optional>
#include <utility>

namespace fs = std::filesystem;

namespace sr {
AsyncTreeWalker::AsyncTreeWalker(std::shared_ptr<kocoro::SynchronousExecutor> synchronousExecutor)
    : asyncDataBase{synchronousExecutor->makeAsync<DataBasePtr>()}
    , asyncTreewalker{synchronousExecutor->makeAsync<TreeWalkerPtr>()}
    , asyncNextCard{synchronousExecutor->makeAsync<CardMeta>()}
{
    synchronousExecutor->startCoro(taskFullfillPromises());
}

auto AsyncTreeWalker::getDataBase() const -> kocoro::Async<DataBasePtr>&
{
    return *asyncDataBase;
}

auto AsyncTreeWalker::getTreeWalker() const -> kocoro::Async<TreeWalkerPtr>&
{
    asyncNextCard->runAsync([_treeWalker = std::move(treeWalker)]() {
        return _treeWalker->getNextCardChoice();
    });
    return *asyncTreewalker;
}

auto AsyncTreeWalker::getNextCardChoice(std::optional<CardId> preferedCardId) -> kocoro::Async<CardMeta>&
{
    asyncNextCard->reset();
    if (treeWalker) {
        asyncNextCard->runAsync([_treeWalker = treeWalker, _preferedCardId = preferedCardId]() {
            return _treeWalker->getNextCardChoice(_preferedCardId);
        });
    }
    return *asyncNextCard;
}

auto AsyncTreeWalker::get_zikhron_cfg() -> std::shared_ptr<zikhron::Config>
{
    auto path_to_exe = fs::read_symlink("/proc/self/exe");
    auto zikhron_cfg_json = path_to_exe.parent_path() / "config.json";
    return std::make_shared<zikhron::Config>(path_to_exe.parent_path());
}

auto AsyncTreeWalker::taskFullfillPromises() -> kocoro::Task<>
{
    asyncDataBase->runAsync([]() -> DataBasePtr {
        auto zikhron_cfg = get_zikhron_cfg();
        auto db = std::make_shared<DataBase>(zikhron_cfg);
        return db;
    });

    DataBasePtr dbPtr = co_await *asyncDataBase;
    asyncTreewalker->runAsync([=]() -> TreeWalkerPtr {
        return ITreeWalker::createTreeWalker(std::move(dbPtr));
    });
    treeWalker = co_await getTreeWalker();
    asyncNextCard->runAsync([_treeWalker = treeWalker]() {
        return _treeWalker->getNextCardChoice();
    });
    co_return;
}

} // namespace sr
