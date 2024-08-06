#include "VideoDB.h"

#include "IdGenerator.h"
#include "VideoSet.h"

#include <misc/Config.h>
#include <spdlog/spdlog.h>
#include <utils/format.h>

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace database {

VideoDB::VideoDB(std::shared_ptr<zikhron::Config> config,
                 std::shared_ptr<PackIdGenerator> _packIdGenerator)
    : packIdGenerator{std::move(_packIdGenerator)}
    , videoSetDir{config->DatabaseDirectory() / s_videoSubdirectory}
    , videoSets{loadVideoSets(videoSetDir, packIdGenerator)}

{
}

auto VideoDB::addVideoSet(const std::vector<std::filesystem::path>& videoFiles) -> VideoSetPtr
{
    if (videoFiles.empty() || !std::filesystem::exists(videoFiles.front())) {
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
    auto videoSet = std::make_shared<VideoSet>(packFilename, packName, videoFiles, packIdGenerator);
    videoSets.push_back(videoSet);
    videoSet->save();

    return videoSet;
}

auto VideoDB::getVideoSets() const -> const std::vector<VideoSetPtr>&
{
    return videoSets;
}

void VideoDB::save()
{
    for (const auto& videoSet : videoSets) {
        videoSet->save();
    }
    spdlog::info("Saved VideoDB");
}

auto VideoDB::loadVideoSets(const std::filesystem::path& directory,
                            const std::shared_ptr<PackIdGenerator>& packIdGenerator)
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
                      [packIdGenerator](const std::filesystem::path& videoSetFile) -> VideoSetPtr {
                          return std::make_shared<VideoSet>(videoSetFile, packIdGenerator);
                      });
    return videoSets;
}
} // namespace database
