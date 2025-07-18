#include "VideoDB.h"

#include "CbdFwd.h"
#include "IdGenerator.h"
#include "VideoSet.h"
#include "WordDB.h"

#include <Video.h>
#include <annotation/Tokenizer.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <misc/Language.h>
#include <spdlog/spdlog.h>
#include <utils/format.h>

#include <algorithm>
#include <filesystem>
#include <generator>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace database {

VideoDB::VideoDB(std::shared_ptr<zikhron::Config> config,
                 Language language,
                 std::shared_ptr<PackIdGenerator> _packIdGenerator,
                 std::shared_ptr<CardIdGenerator> _cardIdGenerator,
                 std::shared_ptr<annotation::Tokenizer> _tokenizer,
                 std::shared_ptr<WordDB> _wordDB)
    : packIdGenerator{std::move(_packIdGenerator)}
    , cardIdGenerator{std::move(_cardIdGenerator)}
    , tokenizer{std::move(_tokenizer)}
    , wordDB{std::move(_wordDB)}
    , videoSetDir{config->DatabaseDirectory() / languageToVideoDirectory.at(language)}
    , videoSets{loadVideoSets(videoSetDir, packIdGenerator, cardIdGenerator, tokenizer, wordDB)}
{
    for (const auto& videoSet : videoSets) {
        addVideosFromVideoSet(videoSet);
    }
}

auto VideoDB::getDeserializedCards() const -> std::generator<CardPtr>
{
    for (const auto& videoSet : videoSets) {
        for (const auto& [_, video] : videoSet->getVideos()) {
            for (const auto& card : video->getActiveSubtitle()->getDeserializedActiveCards()) {
                co_yield card;
            }
        }
    }
    co_return;
}

auto VideoDB::addVideoSet(const std::vector<std::filesystem::path>& videoFiles) -> VideoSetPtr
{
    if (videoFiles.empty() || !std::filesystem::exists(videoFiles.front())) {
        if (!videoFiles.empty()) {
            spdlog::error("File: `{}` doesn't exist", videoFiles.front().c_str());
        } else {
            spdlog::error("No filename provided");
        }
        return nullptr;
    }
    const auto& firstVideoFile = videoFiles.front();
    const auto& parentPath = firstVideoFile.parent_path();

    std::string packName;
    if (videoFiles.size() == 1) {
        packName = videoFiles.front().stem();
    } else {
        packName = parentPath.empty()
                           ? firstVideoFile.stem()
                           : *std::prev(parentPath.end());
    }
    auto packFilename = videoSetDir / std::filesystem::path{packName + s_videoSetExtension};
    auto videoSet = std::make_shared<VideoSet>(packFilename,
                                               packName,
                                               videoFiles,
                                               packIdGenerator,
                                               cardIdGenerator,
                                               tokenizer,
                                               wordDB);
    videoSets.push_back(videoSet);
    videoSet->save();

    addVideosFromVideoSet(videoSet);
    return videoSet;
}

auto VideoDB::getVideoSets() const -> const std::vector<VideoSetPtr>&
{
    return videoSets;
}

auto VideoDB::getVideos() const -> const std::map<PackId, VideoPtr>&
{
    return videos;
}

auto VideoDB::getVideo(const std::string& videoName) const -> VideoPtr
{
    return nameToVideos.at(videoName);
}

void VideoDB::save()
{
    for (const auto& videoSet : videoSets) {
        videoSet->save();
    }
    spdlog::info("Saved VideoDB - nothing to save yet");
}

void VideoDB::saveProgress()
{
    for (const auto& videoSet : videoSets) {
        videoSet->saveProgress();
    }
    spdlog::info("Saved progress videoDB");
}

auto VideoDB::loadVideoSets(const std::filesystem::path& directory,
                            const std::shared_ptr<PackIdGenerator>& packIdGenerator,
                            const std::shared_ptr<CardIdGenerator>& cardIdGenerator,
                            std::shared_ptr<annotation::Tokenizer> tokenizer,
                            std::shared_ptr<WordDB> wordDB)
        -> std::vector<VideoSetPtr>
{
    if (!std::filesystem::exists(directory)) {
        return {};
    }
    std::set<std::filesystem::path> videoSetFiles;
    ranges::copy_if(std::filesystem::directory_iterator(directory), std::inserter(videoSetFiles, videoSetFiles.begin()),
                    [](const std::filesystem::path& file) -> bool { return file.extension() == s_videoSetExtension; });

    std::vector<VideoSetPtr> videoSets;
    ranges::transform(videoSetFiles, std::back_inserter(videoSets),
                      [&](const std::filesystem::path& videoSetFile) -> VideoSetPtr {
                          return std::make_shared<VideoSet>(videoSetFile,
                                                            packIdGenerator,
                                                            cardIdGenerator,
                                                            tokenizer,
                                                            wordDB);
                      });
    return videoSets;
}

void VideoDB::addVideosFromVideoSet(const VideoSetPtr& videoSet)
{
    ranges::copy(videoSet->getVideos(), std::inserter(videos, videos.begin()));
    ranges::transform(videoSet->getVideos(), std::inserter(nameToVideos, nameToVideos.begin()),
                      [](const auto& idVideo) -> std::pair<std::string, VideoPtr> {
                          const auto& [_, video] = idVideo;
                          return {video->getVideoFile().stem(), video};
                      });
}
} // namespace database
