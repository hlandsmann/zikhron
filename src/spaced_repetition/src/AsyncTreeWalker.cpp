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
#include <dictionary/Dictionary.h>
#include <dictionary/DictionaryChi.h>
#include <dictionary/DictionaryJpn.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <misc/Language.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <boost/di.hpp>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <iostream>
#include <kocoro/kocoro.hpp>
#include <memory>
#include <optional>
#include <stacktrace>
#include <utility>

namespace fs = std::filesystem;
namespace ranges = std::ranges;

static auto get_zikhron_cfg() -> std::shared_ptr<zikhron::Config>
{
    auto path_to_exe = fs::read_symlink("/proc/self/exe");
    auto zikhron_cfg = std::make_shared<zikhron::Config>(path_to_exe.parent_path());
    annotation::adaptJiebaDictionaries(zikhron_cfg);
    return zikhron_cfg;
}

namespace sr {
AsyncTreeWalker::AsyncTreeWalker(std::shared_ptr<kocoro::SynchronousExecutor> synchronousExecutor)
// : asyncDataBaseChi{synchronousExecutor->makeAsync<DataBasePtr>()}
// , asyncDataBaseJpn{synchronousExecutor->makeAsync<DataBasePtr>()}
// , asyncTreewalkerChi{synchronousExecutor->makeAsync<TreeWalkerPtr>()}
// , asyncTreewalkerJpn{synchronousExecutor->makeAsync<TreeWalkerPtr>()}
// , asyncNextCardChi{synchronousExecutor->makeAsync<CardMeta>()}
// , asyncNextCardJpn{synchronousExecutor->makeAsync<CardMeta>()}
{
    ranges::generate(asyncDataBaseArray, [&]() { return synchronousExecutor->makeAsync<DataBasePtr>(); });
    ranges::generate(asyncTreeWalkerArray, [&]() { return synchronousExecutor->makeAsync<TreeWalkerPtr>(); });
    ranges::generate(asyncNextCardArray, [&]() { return synchronousExecutor->makeAsync<CardMeta>(); });
    synchronousExecutor->startCoro(taskFullfillPromises());

    // asyncDataBase->runAsync([]() -> DataBasePtr {
    // auto injectorChi = boost::di::make_injector(
    //         boost::di::bind<zikhron::Config>.to(get_zikhron_cfg()),
    //         boost::di::bind<annotation::Tokenizer>.to<annotation::TokenizerChi>(),
    //         boost::di::bind<database::TokenizationChoiceDB>.to<database::TokenizationChoiceDbChi>(),
    //         boost::di::bind<dictionary::Dictionary>.to<dictionary::DictionaryChi>(),
    //         boost::di::bind<Language>.to(Language::chinese));
    // auto injectorJpn = boost::di::make_injector(
    //         boost::di::bind<zikhron::Config>.to(get_zikhron_cfg()),
    //         boost::di::bind<annotation::Tokenizer>.to<annotation::TokenizerJpn>(),
    //         boost::di::bind<dictionary::Dictionary>.to<dictionary::DictionaryJpn>(),
    //         boost::di::bind<Language>.to(Language::japanese));
    // auto db = injectorJpn.create<std::shared_ptr<DataBase>>();
    // asyncDataBase->runAsync([=]() -> DataBasePtr { return db; });
}

auto AsyncTreeWalker::getDataBase(Language language) const -> kocoro::Async<DataBasePtr>&
{
    // switch (language) {
    // case Language::chinese:
    //     return *asyncDataBaseChi;
    // case Language::japanese:
    //     return *asyncDataBaseJpn;
    // }
    return *asyncDataBaseArray.at(static_cast<std::size_t>(language));
}

auto AsyncTreeWalker::getTreeWalker(Language language) const -> kocoro::Async<TreeWalkerPtr>&
{
    // switch (language) {
    // case Language::chinese:
    //     return *asyncTreewalkerChi;
    // case Language::japanese:
    //     return *asyncTreewalkerJpn;
    // }
    return *asyncTreeWalkerArray.at(static_cast<std::size_t>(language));
}

auto AsyncTreeWalker::getNextCardChoice(Language language) -> kocoro::Task<CardMeta>
{
    auto& treeWalker = treeWalkerArray.at(static_cast<std::size_t>(language));
    if (!treeWalker) {
        treeWalker = co_await getTreeWalker(language);
    }
    auto& asyncNextCard = asyncNextCardArray.at(static_cast<std::size_t>(language));
    asyncNextCard->reset();
    asyncNextCard->runAsync([_treeWalker = treeWalker]() {
        return _treeWalker->getNextCardChoice();
    });
    co_return co_await *asyncNextCard;
}

auto AsyncTreeWalker::taskFullfillPromises() -> kocoro::Task<>
{
  auto& asyncDataBaseChi = asyncDataBaseArray.at(static_cast<std::size_t>(Language::chinese));
  auto& asyncDataBaseJpn = asyncDataBaseArray.at(static_cast<std::size_t>(Language::japanese));
    asyncDataBaseChi->runAsync([]() -> DataBasePtr {
        auto injectorChi = boost::di::make_injector(
                boost::di::bind<zikhron::Config>.to(get_zikhron_cfg()),
                boost::di::bind<annotation::Tokenizer>.to<annotation::TokenizerChi>(),
                boost::di::bind<database::TokenizationChoiceDB>.to<database::TokenizationChoiceDbChi>(),
                boost::di::bind<dictionary::Dictionary>.to<dictionary::DictionaryChi>(),
                boost::di::bind<Language>.to(Language::chinese));
        try {
            auto db = injectorChi.create<std::shared_ptr<DataBase>>();
            return db;
        } catch (const std::exception& e) {
            std::cout << std::stacktrace::current();
            spdlog::critical("async, exception: {}", e.what());
            std::terminate();
        }
    });
    asyncDataBaseJpn->runAsync([]() -> DataBasePtr {
        auto injectorJpn = boost::di::make_injector(
                boost::di::bind<zikhron::Config>.to(get_zikhron_cfg()),
                boost::di::bind<annotation::Tokenizer>.to<annotation::TokenizerJpn>(),
                boost::di::bind<dictionary::Dictionary>.to<dictionary::DictionaryJpn>(),
                boost::di::bind<Language>.to(Language::japanese));
        try {
            auto db = injectorJpn.create<std::shared_ptr<DataBase>>();
            return db;
        } catch (const std::exception& e) {
            std::cout << std::stacktrace::current();
            spdlog::critical("async, exception: {}", e.what());
            std::terminate();
        }
    });
    DataBasePtr dbPtrChi = co_await *asyncDataBaseChi;
    auto& asyncTreewalkerChi = asyncTreeWalkerArray.at(static_cast<std::size_t>(Language::chinese));
    asyncTreewalkerChi->runAsync([=]() -> TreeWalkerPtr {
        return ITreeWalker::createTreeWalker(std::move(dbPtrChi));
    });
    DataBasePtr dbPtrJpn = co_await *asyncDataBaseJpn;
    auto& asyncTreewalkerJpn = asyncTreeWalkerArray.at(static_cast<std::size_t>(Language::japanese));
    asyncTreewalkerJpn->runAsync([=]() -> TreeWalkerPtr {
        return ITreeWalker::createTreeWalker(std::move(dbPtrJpn));
    });
    co_return;
}

} // namespace sr
