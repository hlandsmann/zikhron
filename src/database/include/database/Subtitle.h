#pragma once
#include <multimedia/Subtitle.h>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace database {

using SubText = multimedia::SubText;

class Subtitle
{
    static constexpr auto s_subtitleExtension = ".zsub";
    static constexpr auto s_subtitleSubDirectory = "sub";
    static constexpr auto s_type = "zsubtitle";

public:
    Subtitle(const multimedia::Subtitle& sub,
             const std::filesystem::path& videoFile,
             const std::filesystem::path& videoSetDir);
    Subtitle(const std::filesystem::path& subtitleFile,
             const std::filesystem::path& videoSetDir);
    Subtitle(std::filesystem::path subtitleFile);
    [[nodiscard]] auto getName() const -> std::string;
    [[nodiscard]] auto getFileName() const -> std::filesystem::path;
    [[nodiscard]] auto getSubTexts() const -> const std::vector<SubText>&;
    void save();

private:
    void deserialize();
    [[nodiscard]] auto serialize() const -> std::string;
    void cleanSubTexts();
    static auto nameFromSub(const multimedia::Subtitle& sub) -> std::string;
    static auto fileNameFromSubVideo(const multimedia::Subtitle& sub,
                                     const std::filesystem::path& videoFile,
                                     const std::filesystem::path& videoSetDir) -> std::filesystem::path;
    std::string name;
    std::filesystem::path filename;
    std::vector<SubText> subTexts;
};

using SubtitlePtr = std::shared_ptr<Subtitle>;

} // namespace database
