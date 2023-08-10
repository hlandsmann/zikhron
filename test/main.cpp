#include <misc/Config.h>
#include <spaced_repetition/TreeWalker.h>
#include <spaced_repetition/WalkableData.h>
#include <spdlog/spdlog.h>

#include <filesystem>

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
    auto walkableData = std::make_shared<WalkableData>(zikhron_cfg);
    return 0;
}
