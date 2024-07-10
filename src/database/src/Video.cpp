#include "Video.h"

#include <utils/format.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <utility>

namespace database {

Video::Video(std::string_view sv)
{}

Video::Video(std::filesystem::path _videoFile)
    : videoFile{std::move(_videoFile)}
{}

auto Video::serialize() const -> std::string
{
    std::string content;
    content += fmt::format("vid:{}\n", videoFile.string());
    return content;
}

} // namespace database
