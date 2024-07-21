#pragma once

#include <filesystem>
#include <string_view>

namespace zikhron {
class Config
{
    static constexpr std::string_view s_database_directory = "database_directory";
    static constexpr std::string_view s_dictionary = "dictionary";
    std::filesystem::path configFilename = "config.json";

public:
    Config();
    Config(std::filesystem::path configDirectory);
    Config(Config&&) = delete;
    Config(const Config&) = delete;
    auto operator=(Config&&) -> Config = delete;
    auto operator=(const Config&) -> Config = delete;
    virtual ~Config() = default;

    static auto ConfigDirectory() -> std::filesystem::path;
    [[nodiscard]] auto Dictionary() const -> const std::filesystem::path&;
    [[nodiscard]] auto DatabaseDirectory() const -> const std::filesystem::path&;

private:
    void loadJsonFile(std::filesystem::path configFile);

    std::filesystem::path configDirectory;

    std::filesystem::path dictionary;
    std::filesystem::path databaseDirectory;
};
} // namespace zikhron
