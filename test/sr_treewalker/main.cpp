#include <annotation/TextCard.h>
#include <dictionary/ZH_Dictionary.h>
#include <spaced_repetition/TreeWalker.h>
#include <spdlog/spdlog.h>
#include <cstdlib>
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

auto loadCardDB(const fs::path& card_db_path) ->std::shared_ptr< CardDB >{
    auto cardDB=std::make_shared<CardDB>();
    try {
        cardDB->loadFromDirectory(card_db_path / "cards");
    }
    catch (const std::exception& e) {
        spdlog::error("Exception: {}", e.what());
        std::quick_exit(1);
    }
    catch (...) {
        spdlog::error("Unknown Error, load Card Database failed!");
        std::quick_exit(1);
    }
    return cardDB;
}

int main() {
    auto zikhron_cfg = get_zikhron_cfg();
    fs::path dictionary_fn = zikhron_cfg["dictionary"];
    fs::path card_db_path = zikhron_cfg["card_db"];

    auto zh_dictionary = std::make_shared<ZH_Dictionary>(dictionary_fn);
    auto cardDB = loadCardDB(card_db_path);
    spdlog::info("CardDB size: {}", cardDB->get().size());
    TreeWalker treeWalker{cardDB, zh_dictionary};
}
