#include "Subtitle.h"

#include "ParsingHelpers.h"

#include <multimedia/ExtractSubtitles.h>
#include <multimedia/Subtitle.h>
#include <utils/Crc32.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <exception>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace database {
Subtitle::Subtitle(const multimedia::Subtitle& sub,
                   const std::filesystem::path& videoFile,
                   const std::filesystem::path& videoSetDir)
    : name{nameFromSub(sub)}
    , filename{fileNameFromSubVideo(sub, videoFile, videoSetDir)}
    , subTexts{sub.subs}
{
}

Subtitle::Subtitle(const std::filesystem::path& subtitleFile,
                   const std::filesystem::path& videoSetDir)
    : filename{videoSetDir / s_subtitleSubDirectory / subtitleFile}
{
    try {
        deserialize();
    } catch (const std::exception& e) {
        spdlog::error("Failed to parse {}", subtitleFile.string());
        spdlog::error("{}", e.what());
    }
    cleanSubTexts();
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
                                    const std::filesystem::path& videoSetDir) -> std::filesystem::path
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
    return videoSetDir / s_subtitleSubDirectory / fileName;
}

auto Subtitle::getSubTexts() const -> const std::vector<SubText>&
{
    return subTexts;
}

void Subtitle::save()
{
    std::filesystem::create_directories(filename.parent_path());
    auto out = std::ofstream{filename};
    out << serialize();
}

void Subtitle::deserialize()
{
    auto content = utl::load_string_file(filename);
    auto rest = std::string_view{content};
    verifyFileType(rest, s_type);
    auto version = getValue(rest, "version");
    if (version != "1.0") {
        throw std::runtime_error(fmt::format("Only version 1.0 is supported, got: {}", version));
    }
    name = getValue(rest, "name");
    std::string oldSubtextStr;
    while (!rest.empty()) {
        auto subtextSV = utl::split_front(rest, "\n;\n");
        auto subtextStr = std::string{subtextSV};
        try {
            auto metaSV = getValue(subtextSV, "meta");
            subTexts.push_back({.startTime = std::stol(std::string{utl::split_front(metaSV, ',')}),
                                .duration = std::stol(std::string{utl::split_front(metaSV, ',')}),
                                .style = std::string{metaSV},
                                .text = std::string{getValue(subtextSV, "text")}});
        } catch (const std::exception& e) {
            fmt::print("content:`{}`\n", subtextStr);
            fmt::print("old_content:`{}`\n", oldSubtextStr);
            throw e;
        }
        oldSubtextStr = subtextStr;
    }
}

auto Subtitle::serialize() const -> std::string
{
    std::string content;
    content += fmt::format("{};version:1.0\n", s_type);
    content += fmt::format("name:{}\n", name);

    for (const auto& subText : subTexts) {
        content += fmt::format("meta:{},{},{}\n", subText.startTime, subText.duration, subText.style);
        content += fmt::format("text:{}\n\n;\n", subText.text);
    }

    return content;
}

void Subtitle::cleanSubTexts()
{
    for (auto& subText : subTexts) {
        auto subStr = std::string{"\\N"};
        auto found = subText.text.find(subStr);
        while (found != std::string::npos) {
            subText.text.replace(found, subStr.length(), " ");
            found = subText.text.find(subStr);
        }
    }
}
} // namespace database
