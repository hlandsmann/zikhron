#include "Video.h"

#include "ParsingHelpers.h"

#include <utils/format.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <utility>

namespace database {

Video::Video(std::string_view sv)
{
    deserialize(sv);
}

Video::Video(std::filesystem::path _videoFile)
    : videoFile{std::move(_videoFile)}
{}

void Video::deserialize(std::string_view content)
{
    auto rest = std::string_view{content};
    videoFile = getValue(rest, "vid");
}

auto Video::serialize() const -> std::string
{
    std::string content;
    content += fmt::format("vid:{}\n", videoFile.string());
    return content;
}

} // namespace database
