#include <AsyncTreeWalker.h>
#include <CardMeta.h>
#include <DataBase.h>
#include <ITreeWalker.h>
#include <annotation/AdaptJiebaDict.h>
#include <annotation/Tokenizer.h>
#include <annotation/TokenizerChi.h>
#include <annotation/TokenizerJpn.h>
#include <database/TokenizationChoiceDB.h>
#include <database/TokenizationChoiceDbChi.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <misc/Language.h>
#include <spdlog/spdlog.h>

#include <boost/di.hpp>
#include <exception>
#include <filesystem>
#include <iostream>
#include <kocoro/kocoro.hpp>
#include <memory>
#include <optional>
#include <stacktrace>
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

auto AsyncTreeWalker::getNextCardChoice() -> kocoro::Async<CardMeta>&
{
    asyncNextCard->reset();
    if (treeWalker) {
        asyncNextCard->runAsync([_treeWalker = treeWalker]() {
            return _treeWalker->getNextCardChoice();
        });
    }
    return *asyncNextCard;
}

auto AsyncTreeWalker::taskFullfillPromises() -> kocoro::Task<>
{
    asyncDataBase->runAsync([]() -> DataBasePtr {
        auto injectorChi = boost::di::make_injector(
                boost::di::bind<zikhron::Config>.to(get_zikhron_cfg()),
                boost::di::bind<annotation::Tokenizer>.to<annotation::TokenizerChi>(),
                boost::di::bind<database::TokenizationChoiceDB>.to<database::TokenizationChoiceDbChi>(),
                boost::di::bind<Language>.to(Language::chinese)

                // boost::di::bind<annotation::Tokenizer>.to([](const auto& injector) -> annotation::Tokenizer{
                //     return injector.template create<annotation::TokenizerChi>();
                // })

        );
        try {
            auto db = injectorChi.create<std::shared_ptr<DataBase>>();
            return db;
        } catch (const std::exception& e) {
            std::cout << std::stacktrace::current();
            spdlog::critical("async, exception: {}", e.what());
            std::terminate();
        }
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
