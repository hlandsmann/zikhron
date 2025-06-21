#include "VideoSet.h"

#include "IdGenerator.h"
#include "ParsingHelpers.h"
#include "Video.h"
#include "WordDB.h"

#include <annotation/Tokenizer.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <utils/Memory.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <algorithm>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace database {

VideoSet::VideoSet(std::filesystem::path _videoSetFile,
                   std::shared_ptr<PackIdGenerator> _packIdGenerator,
                   std::shared_ptr<CardIdGenerator> _cardIdGenerator,
                   std::shared_ptr<annotation::Tokenizer> _tokenizer,
                   std::shared_ptr<WordDB> _wordDB)
    : videoSetFile{std::move(_videoSetFile)}
    , packIdGenerator{std::move(_packIdGenerator)}
    , cardIdGenerator{std::move(_cardIdGenerator)}
    , tokenizer{std::move(_tokenizer)}
    , wordDB{std::move(_wordDB)}
{
    try {
        deserialize();
    } catch (const std::exception& e) {
        spdlog::error("Failed to parse {}", videoSetFile.string());
        spdlog::error("{}", e.what());
    }

    setChoice(choice);
}

VideoSet::VideoSet(std::filesystem::path _videoSetFile,
                   std::string _name,
                   const std::vector<std::filesystem::path>& videoFiles,
                   std::shared_ptr<PackIdGenerator> _packIdGenerator,
                   std::shared_ptr<CardIdGenerator> _cardIdGenerator,
                   std::shared_ptr<annotation::Tokenizer> _tokenizer,
                   std::shared_ptr<WordDB> _wordDB)
    : videoSetFile{std::move(_videoSetFile)}
    , name{std::move(_name)}
    , packIdGenerator{std::move(_packIdGenerator)}
    , cardIdGenerator{std::move(_cardIdGenerator)}
    , tokenizer{std::move(_tokenizer)}
    , wordDB{std::move(_wordDB)}
    , videos{genVideosFromPaths(videoFiles,
                                videoSetFile,
                                packIdGenerator,
                                cardIdGenerator,
                                tokenizer,
                                wordDB)}
{
    setChoice(choice);
}

auto VideoSet::getName() const -> const std::string&
{
    return name;
}

auto VideoSet::getVideos() const -> const std::map<PackId, VideoPtr>&
{
    return videos;
}

void VideoSet::setChoice(std::size_t _choice)
{
    choice = std::min(_choice, videos.size() - 1);
    const auto& [_, videoPtr] = *std::next(videos.begin(), static_cast<int>(choice));
    videoChoice = videoPtr;
}

auto VideoSet::getChoice() const -> std::pair<std::size_t, VideoPtr>
{
    return {choice, videoChoice};
}

auto VideoSet::getCover() const -> std::filesystem::path
{
    return cover;
}

void VideoSet::save()
{
    // fmt::print("{}", serialize());
    std::filesystem::create_directories(videoSetFile.parent_path());
    auto out = std::ofstream{videoSetFile};
    out << serialize();
}

void VideoSet::saveProgress()
{
    for (const auto& [_, video] : videos) {
        video->saveProgress();
    }
}

void VideoSet::deserialize()
{
    auto content = utl::load_string_file(videoSetFile);
    auto rest = std::string_view{content};
    verifyFileType(rest, s_type);

    auto version = getValue(rest, "version");
    if (version != "1.0") {
        throw std::runtime_error(fmt::format("Only version 1.0 is supported, got: {}, fn: {}", version, videoSetFile.string()));
    }

    name = getValue(rest, "name");
    cover = getValue(rest, "cover");
    choice = std::stoul(std::string{getValue(rest, "choice")});

    while (!rest.empty()) {
        auto videoId = packIdGenerator->getNext();
        auto videoSV = utl::split_front(rest, "\n;\n");
        videos[videoId] = std::make_shared<Video>(videoSV,
                                                  videoSetFile,
                                                  videoId,
                                                  cardIdGenerator,
                                                  tokenizer,
                                                  wordDB);
    }
}

auto VideoSet::serialize() const -> std::string
{
    std::string content;
    content += fmt::format("{};version:1.0\n", s_type);
    content += fmt::format("name:{}\n", name);
    content += fmt::format("cover:{}\n", cover.string());
    content += fmt::format("choice:{}\n", choice);

    for (const auto& [videoId, video] : videos) {
        content += fmt::format("{}\n;\n", video->serialize());
    }
    return content;
}

auto VideoSet::genVideosFromPaths(const std::vector<std::filesystem::path>& videoFiles,
                                  const std::filesystem::path& videoSetFile,
                                  std::shared_ptr<PackIdGenerator> packIdGenerator,
                                  std::shared_ptr<CardIdGenerator> cardIdGenerator,
                                  std::shared_ptr<annotation::Tokenizer> tokenizer,
                                  std::shared_ptr<WordDB> wordDB)
        -> std::map<PackId, VideoPtr>
{
    std::map<PackId, VideoPtr> videos;
    ranges::transform(videoFiles, std::inserter(videos, videos.begin()),
                      [&](const std::filesystem::path& videoFile) -> std::pair<PackId, VideoPtr> {
                          auto videoId = packIdGenerator->getNext();
                          return {videoId, std::make_shared<Video>(videoFile,
                                                                   videoSetFile,
                                                                   videoId,
                                                                   cardIdGenerator,
                                                                   tokenizer,
                                                                   wordDB)};
                      });
    return videos;
}
} // namespace database
