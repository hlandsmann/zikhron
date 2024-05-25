#include <Config.h>
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

namespace zikhron {
Config::Config()
    : Config{ConfigDirectory()}
{
}

Config::Config(std::filesystem::path _configDirectory)
    : configDirectory{std::move(_configDirectory)}
{
    loadJsonFile(configDirectory / configFilename);
}

auto Config::ConfigDirectory() -> std::filesystem::path
{
    std::filesystem::path home = std::getenv("HOME");
    std::filesystem::path config_dir = home.empty() ? home / ".config" : "~/.config";
    return config_dir / "zikhron";
}

auto Config::Dictionary() const -> const std::filesystem::path&
{
    return dictionary;
}

auto Config::DatabaseDirectory() const -> const std::filesystem::path&
{
    return databaseDirectory;
}

void Config::loadJsonFile(std::filesystem::path configFile)
{
    std::ifstream ifs(configFile);

    if (not ifs.is_open()) {
        spdlog::error("Failed to open {}", configFile.c_str());
        return;
    }
    auto zikhron_cfg = nlohmann::json::parse(ifs);
    dictionary = zikhron_cfg["dictionary"].get<std::string>();
    databaseDirectory = zikhron_cfg["database_directory"].get<std::string>();
}

} // namespace zikhron
