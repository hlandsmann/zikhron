#include "Video.h"

#include "IdGenerator.h"
#include "ParsingHelpers.h"
#include "Subtitle.h"
#include "SubtitlePicker.h"
#include "WordDB.h"

#include <annotation/Tokenizer.h>
#include <misc/Identifier.h>
#include <multimedia/ExtractSubtitles.h>
#include <multimedia/Subtitle.h>
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

Video::Video(std::string_view sv,
             std::filesystem::path _videoSetFile,
             PackId _videoId,
             std::shared_ptr<CardIdGenerator> _cardIdGenerator,
             std::shared_ptr<annotation::Tokenizer> _tokenizer,
             std::shared_ptr<WordDB> _wordDB)
    : videoSetFile{std::move(_videoSetFile)}
    , videoId{_videoId}
    , cardIdGenerator{std::move(_cardIdGenerator)}
    , tokenizer{std::move(_tokenizer)}
    , wordDB{std::move(_wordDB)}
{
    deserialize(sv);
    createSubtitlePicker();
}

Video::Video(std::filesystem::path _videoFile,
             std::filesystem::path _videoSetFile,
             PackId _videoId,
             std::shared_ptr<CardIdGenerator> _cardIdGenerator,
             std::shared_ptr<annotation::Tokenizer> _tokenizer,
             std::shared_ptr<WordDB> _wordDB)
    : videoSetFile{std::move(_videoSetFile)}
    , videoFile{std::move(_videoFile)}
    , name{videoFile.stem().string()}
    , videoId{_videoId}
    , cardIdGenerator{std::move(_cardIdGenerator)}
    , tokenizer{std::move(_tokenizer)}
    , wordDB{std::move(_wordDB)}
{
    loadSubtitles();
}

void Video::deserialize(std::string_view content)
{
    auto rest = std::string_view{content};
    videoFile = getValue(rest, "vid");
    name = getValue(rest, "name");
    subChoice = std::stoul(std::string{getValue(rest, "sub_choice")});
    while (!rest.empty()) {
        if (getValueType(rest) == "sub") {
            auto subtitleFile = getValue(rest, "sub");
            subtitles.push_back(std::make_shared<Subtitle>(subtitleFile, videoSetFile.parent_path()));
            continue;
        }
        break;
    }
    if (!subtitles.empty()) {
        fmt::print("video: `{}`\n", name);
        fmt::print("sub_choice: `{}`, nsubs: {}\n\n",
                   subtitles.at(subChoice)->getName(),
                   subtitles.at(subChoice)->getSubTexts().size());
    }
}

auto Video::serialize() const -> std::string
{
    std::string content;
    content += fmt::format("vid:{}\n", videoFile.string());
    content += fmt::format("name:{}\n", name);
    content += fmt::format("sub_choice:{}\n", subChoice);

    for (const auto& sub : subtitles) {
        content += fmt::format("sub:{}\n", sub->getFileName().filename().string());
    }
    return content;
}

void Video::loadSubtitles()
{
    auto stopToken = std::stop_token{};
    auto subtitleDecoder = multimedia::ExtractSubtitles{videoFile};
    auto subs = subtitleDecoder.decode(stopToken);
    ranges::transform(subs, std::back_inserter(subtitles),
                      [this](const multimedia::Subtitle sub) -> SubtitlePtr {
                          return std::make_shared<Subtitle>(sub, videoFile, videoSetFile.parent_path());
                      });
    for (const auto& sub : subtitles) {
        fmt::print("fn: `{}`\n", sub->getFileName().string());
        sub->save();
    }
}

void Video::createSubtitlePicker()
{
    auto chosenSubtitle = subtitles.empty() ? std::shared_ptr<Subtitle>{nullptr}
                                            : subtitles.at(subChoice);
    subtitlePicker = std::make_shared<SubtitlePicker>(chosenSubtitle,
                                                      videoId,
                                                      name,
                                                      cardIdGenerator,
                                                      tokenizer,
                                                      wordDB);
}

auto Video::getVideoFile() const -> const std::filesystem::path&
{
    return videoFile;
}

auto Video::getName() const -> std::string
{
    return name;
}

void Video::saveProgress()
{
    if (subtitlePicker != nullptr) {
        subtitlePicker->save();
    }
}

auto Video::getActiveSubtitle() -> SubtitlePickerPtr
{
    if (subtitlePicker != nullptr) {
        return subtitlePicker;
    }
    createSubtitlePicker();
    return subtitlePicker;
}

} // namespace database
