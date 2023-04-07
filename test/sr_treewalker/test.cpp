#include <annotation/TextCard.h>
#include <dictionary/ZH_Dictionary.h>
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

auto get_zikhron_cfg() {
    auto path_to_exe = fs::read_symlink("/proc/self/exe");
    auto zikhron_cfg_json = path_to_exe.parent_path() / "zikhron_cfg.json";

    std::ifstream ifs(zikhron_cfg_json);
    return nlohmann::json::parse(ifs);
}

auto loadCardDB(const fs::path& card_db_path) -> CardDB {
    CardDB cardDB;
    try {
        cardDB.loadFromDirectory(card_db_path / "cards");
    }
    catch (const std::exception& e) {
        FAIL(e.what());
    }
    catch (...) {
        FAIL("Unknown Error, load Card Database failed!");
    }
    return cardDB;
}

TEST_CASE("Tree Walker") {
    auto zikhron_cfg = get_zikhron_cfg();
    fs::path dictionary_fn = zikhron_cfg["dictionary"];
    fs::path card_db_path = zikhron_cfg["card_db"];

    auto zh_dictionary = ZH_Dictionary(dictionary_fn);
    CardDB cardDB = loadCardDB(card_db_path);
}
