#include "VideoSet.h"

#include "ParsingHelpers.h"
#include "Video.h"

#include <spdlog/spdlog.h>
#include <utils/Memory.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace database {

VideoSet::VideoSet(std::filesystem::path _videoSetFile)
    : videoSetFile{std::move(_videoSetFile)}
{
    try {
        deserialize();
    } catch (const std::exception& e) {
        spdlog::error("Failed to parse {}", videoSetFile.string());
        spdlog::error("{}", e.what());
    }
}

VideoSet::VideoSet(std::filesystem::path _videoSetFile,
                     std::string _name,
                     const std::vector<std::filesystem::path>& videoFiles)
    : videoSetFile{std::move(_videoSetFile)}
    , name{std::move(_name)}
    , videos{genVideosFromPaths(videoFiles, videoSetFile)}
{}

auto VideoSet::getName() const -> const std::string&
{
    return name;
}

auto VideoSet::getVideo() const -> VideoPtr
{
    return videos.front();
}

void VideoSet::save()
{
    // fmt::print("{}", serialize());
    std::filesystem::create_directories(videoSetFile.parent_path());
    auto out = std::ofstream{videoSetFile};
    out << serialize();
}

void VideoSet::deserialize()
{
    auto content = utl::load_string_file(videoSetFile);
    auto rest = std::string_view{content};
    verifyFileType(rest, s_type);

    auto version = getValue(rest, "version");
    if (version != "1.0") {
        throw std::runtime_error(fmt::format("Only version 1.0 is supported, got: {}", version));
    }

    name = getValue(rest, "name");

    while (!rest.empty()) {
        auto videoSV = utl::split_front(rest, "\n;\n");
        auto video = std::make_shared<Video>(videoSV, videoSetFile);
        videos.push_back(video);
    }
}

auto VideoSet::serialize() const -> std::string
{
    std::string content;
    content += fmt::format("{};version:1.0\n", s_type);
    content += fmt::format("name:{}\n", name);
    for (const auto& video : videos) {
        content += fmt::format("{}\n;\n", video->serialize());
    }
    return content;
}

auto VideoSet::genVideosFromPaths(const std::vector<std::filesystem::path>& videoFiles, const std::filesystem::path& videoSetFile) -> std::vector<VideoPtr>
{
    std::vector<VideoPtr> videos;
    ranges::transform(videoFiles, std::back_inserter(videos),
                      [&](const std::filesystem::path& videoFile) -> VideoPtr {
                          return std::make_shared<Video>(videoFile, videoSetFile);
                      });
    return videos;
}
} // namespace database
