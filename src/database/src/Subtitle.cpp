#include "Subtitle.h"

#include <multimedia/ExtractSubtitles.h>
#include <utils/Crc32.h>
#include <utils/format.h>

#include <filesystem>
#include <string>

namespace database {
Subtitle::Subtitle(const multimedia::Subtitle& sub,
                   const std::filesystem::path& videoFile,
                   const std::filesystem::path& videoPackDir)
    : name{nameFromSub(sub)}
    , filename{fileNameFromSubVideo(sub, videoFile, videoPackDir)}
{
}

auto Subtitle::getName() const -> std::string
{
    return name;
}

auto Subtitle::getFileName() const -> std::filesystem::path
{
    return filename;
}

auto Subtitle::nameFromSub(const multimedia::Subtitle& sub) -> std::string
{
    std::string name;
    if (sub.language.empty() && sub.title.empty()) {
        name += fmt::format("track: {}", sub.indexInVideo);
    } else if (sub.language.empty() || sub.title.empty()) {
        name += fmt::format("{}{}", sub.language, sub.title);
    } else {
        name += fmt::format("{}: {}", sub.language, sub.title);
    }
    return name;
}

auto Subtitle::fileNameFromSubVideo(const multimedia::Subtitle& sub,
                                    const std::filesystem::path& videoFile,
                                    const std::filesystem::path& videoPackDir) -> std::filesystem::path
{
    std::string allowedCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789%~#%-_+,.";
    std::string subName = nameFromSub(sub);
    auto nameCRC = utl::calculateCrc32(videoFile.string() + subName);
    for (char& character : subName) {
        if (!allowedCharacters.contains(character)) {
            character = '.';
        }
    }
    auto fileName = fmt::format("{}.{}.{:08x}{}", videoFile.stem().string(), subName, nameCRC, s_subtitleExtension);
    return videoPackDir.parent_path() / s_subtitleSubDirectory / fileName;
}
} // namespace database
