#include "Video.h"

#include "ParsingHelpers.h"

#include <Subtitle.h>
#include <multimedia/ExtractSubtitles.h>
#include <utils/format.h>

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <memory>
#include <stop_token>
#include <string>
#include <string_view>
#include <utility>

namespace ranges = std::ranges;

namespace database {

Video::Video(std::string_view sv, std::filesystem::path _videoPackFile)
    : videoPackFile{std::move(_videoPackFile)}
{
    deserialize(sv);
    subtitleDecoder = std::make_unique<multimedia::ExtractSubtitles>(videoFile);
    loadSubtitles();
}

Video::Video(std::filesystem::path _videoFile, std::filesystem::path _videoPackFile)
    : videoPackFile{std::move(_videoPackFile)}
    , videoFile{std::move(_videoFile)}
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
    auto subs = subtitleDecoder->decode(stopToken);
    ranges::transform(subs, std::back_inserter(subtitles),
                      [this](const multimedia::Subtitle sub) -> SubtitlePtr {
                          return std::make_shared<Subtitle>(sub, videoFile, videoPackFile);
                      });
    for (const auto& sub : subtitles) {
        fmt::print("fn: `{}`\n", sub->getFileName().string());
    }
}

} // namespace database
