#pragma once
#include "CardMeta.h"
#include "DataBase.h"
#include "ITreeWalker.h"

#include <misc/Config.h>
#include <misc/Identifier.h>
#include <misc/Language.h>

#include <array>
#include <cstddef>
#include <kocoro/kocoro.hpp>
#include <memory>

namespace sr {
class AsyncTreeWalker
{
public:
    using DataBasePtr = std::shared_ptr<DataBase>;
    using TreeWalkerPtr = std::shared_ptr<ITreeWalker>;
    AsyncTreeWalker(std::shared_ptr<kocoro::SynchronousExecutor> synchronousExecutor);

    [[nodiscard]] auto getDataBase(Language language) const -> kocoro::Async<DataBasePtr>&;
    [[nodiscard]] auto getTreeWalker(Language language) const -> kocoro::Async<TreeWalkerPtr>&;
    auto getNextCardChoice(Language language) -> kocoro::Task<CardMeta>;

private:
    static auto taskCreateDataBase() -> kocoro::Task<DataBasePtr>;
    auto taskFullfillPromises() -> kocoro::Task<>;

    constexpr static std::size_t languageCount = static_cast<std::size_t>(Language::languageCount);
    std::array<kocoro::AsyncPtr<DataBasePtr>, languageCount> asyncDataBaseArray;
    // kocoro::AsyncPtr<DataBasePtr> asyncDataBaseChi;
    // kocoro::AsyncPtr<DataBasePtr> asyncDataBaseJpn;
    std::array<kocoro::AsyncPtr<TreeWalkerPtr>, languageCount> asyncTreeWalkerArray;
    // kocoro::AsyncPtr<TreeWalkerPtr> asyncTreewalkerChi;
    // kocoro::AsyncPtr<TreeWalkerPtr> asyncTreewalkerJpn;
    std::array<kocoro::AsyncPtr<CardMeta>, languageCount> asyncNextCardArray;
    // kocoro::AsyncPtr<CardMeta> asyncNextCardChi;
    // kocoro::AsyncPtr<CardMeta> asyncNextCardJpn;

    std::array<TreeWalkerPtr, languageCount> treeWalkerArray;
    // TreeWalkerPtr treeWalkerChi;
    // TreeWalkerPtr treeWalkerJpn;
};
} // namespace sr
