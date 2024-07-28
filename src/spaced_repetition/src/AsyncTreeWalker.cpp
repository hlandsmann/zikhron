#include <AsyncTreeWalker.h>
#include <CardMeta.h>
#include <DataBase.h>
#include <ITreeWalker.h>
#include <annotation/AdaptJiebaDict.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>

#include <boost/di.hpp>
#include <filesystem>
#include <kocoro/kocoro.hpp>
#include <memory>
#include <optional>
#include <utility>

namespace fs = std::filesystem;

static auto get_zikhron_cfg() -> std::shared_ptr<zikhron::Config>
{
    auto path_to_exe = fs::read_symlink("/proc/self/exe");
    auto zikhron_cfg_json = path_to_exe.parent_path() / "config.json";

    auto zikhron_cfg = std::make_shared<zikhron::Config>(path_to_exe.parent_path());
    annotation::adaptJiebaDictionaries(zikhron_cfg);
    return zikhron_cfg;
}

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

auto AsyncTreeWalker::taskFullfillPromises() -> kocoro::Task<>
{
    asyncDataBase->runAsync([]() -> DataBasePtr {
        auto injector = boost::di::make_injector(
                boost::di::bind<zikhron::Config>.to(get_zikhron_cfg()));

        auto db = injector.create<std::shared_ptr<DataBase>>();
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
