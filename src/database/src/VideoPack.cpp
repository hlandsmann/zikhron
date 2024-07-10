#include "VideoPack.h"

#include "Video.h"

#include <utils/Memory.h>
#include <utils/format.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace database {

VideoPack::VideoPack(std::filesystem::path _videoPackFile)
    : videoPackFile{std::move(_videoPackFile)}
{}

VideoPack::VideoPack(std::filesystem::path _videoPackFile,
                     std::string _name,
                     const std::vector<std::filesystem::path>& videoFiles)
    : videoPackFile{std::move(_videoPackFile)}
    , name{std::move(_name)}
    , videos{genVideosFromPaths(videoFiles)}
{}

auto VideoPack::getName() const -> const std::string&
{
    return name;
}

void VideoPack::save()
{
    fmt::print("{}", serialize());
    std::filesystem::create_directories(videoPackFile.parent_path());
    auto out = std::ofstream{videoPackFile};
    out << serialize();
}

auto VideoPack::serialize() const -> std::string
{
    std::string content;
    content += fmt::format("{};version:1.0\n", s_type);
    content += fmt::format("name:{}\n", name);
    for (const auto& video : videos) {
        content += fmt::format("{}\n;\n", video->serialize());
    }
    return content;
}

auto VideoPack::genVideosFromPaths(const std::vector<std::filesystem::path>& videoFiles) -> std::vector<VideoPtr>
{
    std::vector<VideoPtr> videos;
    ranges::transform(videoFiles, std::back_inserter(videos),
                      [](const std::filesystem::path& videoFile) -> VideoPtr {
                          return std::make_shared<Video>(videoFile);
                      });
    return videos;
}
} // namespace database
