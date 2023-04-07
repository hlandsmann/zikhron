#include <dictionary/ZH_Dictionary.h>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include <catch2/catch_all.hpp>

namespace fs = std::filesystem;

auto get_dictionary_filename() -> fs::path {
    auto path_to_exe = fs::read_symlink("/proc/self/exe");
    auto zikhron_cfg_json = path_to_exe.parent_path() / "zikhron_cfg.json";

    std::ifstream ifs(zikhron_cfg_json);
    auto zikhron_cfg = nlohmann::json::parse(ifs);
    fs::path dictionary = zikhron_cfg["dictionary"];

    return dictionary;
}

TEST_CASE("Dictionary loading") {
    BENCHMARK_ADVANCED("dictionary")(Catch::Benchmark::Chronometer meter) {
        auto dictionary = get_dictionary_filename();
        meter.measure([&dictionary] { return ZH_Dictionary(dictionary); });
    };
}
