#pragma once
#include <multimedia/ExtractSubtitles.h>

#include <filesystem>
#include <memory>
#include <string>

namespace database {

class Subtitle
{
    static constexpr auto s_subtitleExtension = ".zsub";
    static constexpr auto s_subtitleSubDirectory = "sub";

public:
    Subtitle(const multimedia::Subtitle& sub,
             const std::filesystem::path& videoFile,
             const std::filesystem::path& videoPackDir);
    [[nodiscard]] auto getName() const -> std::string;
    [[nodiscard]] auto getFileName() const -> std::filesystem::path;

private:
    static auto nameFromSub(const multimedia::Subtitle& sub) -> std::string;
    static auto fileNameFromSubVideo(const multimedia::Subtitle& sub,
                                     const std::filesystem::path& videoFile,
                                     const std::filesystem::path& videoPackDir) -> std::filesystem::path;
    std::string name;
    std::filesystem::path filename;
};

using SubtitlePtr = std::shared_ptr<Subtitle>;

} // namespace database
