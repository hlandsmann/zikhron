#include "Video.h"

#include "ParsingHelpers.h"

#include <multimedia/ExtractSubtitles.h>
#include <utils/format.h>

#include <filesystem>
#include <memory>
#include <stop_token>
#include <string>
#include <string_view>
#include <utility>

namespace database {

Video::Video(std::string_view sv)
{
    deserialize(sv);
    subtitleDecoder = std::make_unique<multimedia::ExtractSubtitles>(videoFile);
    loadSubtitles();
}

Video::Video(std::filesystem::path _videoFile)
    : videoFile{std::move(_videoFile)}
    , subtitleDecoder{std::make_unique<multimedia::ExtractSubtitles>(videoFile)}
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

void Video::loadSubtitles()
{
    auto stopToken = std::stop_token{};
    subtitleDecoder->decode(stopToken);
}

} // namespace database
