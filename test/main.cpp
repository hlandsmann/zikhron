#include <misc/Config.h>
#include <spaced_repetition/ITreeWalker.h>
#include <spaced_repetition/DataBase.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <memory>
#include <utility>

namespace fs = std::filesystem;

auto get_zikhron_cfg() -> std::shared_ptr<zikhron::Config>
{
    auto path_to_exe = fs::read_symlink("/proc/self/exe");
    auto zikhron_cfg_json = path_to_exe.parent_path() / "config.json";
    return std::make_shared<zikhron::Config>(path_to_exe.parent_path());
}

auto main() -> int
{
    auto zikhron_cfg = get_zikhron_cfg();
    auto db = std::make_unique<sr::DataBase>(zikhron_cfg);
    auto treeWalker = sr::ITreeWalker::createTreeWalker(std::move(db));
    auto [optCardId, _, ease] = treeWalker->getNextCardChoice();
    if (not optCardId.has_value()) {
        spdlog::info("No card found!");
        return 0;
    }
    auto cardId = optCardId.value()->Id();
    spdlog::info("show cardId: {}, size: {}", cardId, ease.size());

    return 0;
}
