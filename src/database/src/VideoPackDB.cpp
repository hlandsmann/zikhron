#include "VideoPackDB.h"

#include <VideoPack.h>
#include <misc/Config.h>
#include <spdlog/spdlog.h>
#include <utils/format.h>

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace ranges = std::ranges;

namespace database {

VideoPackDB::VideoPackDB(std::shared_ptr<zikhron::Config> config)
    : videoPackDir{config->DatabaseDirectory() / s_videoSubdirectory}
    , videoPacks{loadVideoPacks(videoPackDir)}

{
}

auto VideoPackDB::addVideoPack(const std::vector<std::filesystem::path>& videoFiles) -> VideoPackPtr
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
    auto packFilename = videoPackDir / std::filesystem::path{packName + s_videoPackExtension};
    auto videoPack = std::make_shared<VideoPack>(packFilename, packName, videoFiles);
    videoPacks.push_back(videoPack);
    videoPack->save();

    return videoPack;
}

auto VideoPackDB::getVideoPacks() const -> const std::vector<VideoPackPtr>&
{
    return videoPacks;
}

void VideoPackDB::save()
{
    for (const auto& videoPack : videoPacks) {
        videoPack->save();
    }
}

auto VideoPackDB::loadVideoPacks(const std::filesystem::path& directory) -> std::vector<VideoPackPtr>
{
    if (!std::filesystem::exists(directory)) {
        return {};
    }
    std::set<std::filesystem::path> videoPackFiles;
    ranges::copy_if(std::filesystem::directory_iterator(directory), std::inserter(videoPackFiles, videoPackFiles.begin()),
                    [](const std::filesystem::path& file) -> bool { return file.extension() == s_videoPackExtension; });

    std::vector<VideoPackPtr> videoPacks;
    ranges::transform(videoPackFiles, std::back_inserter(videoPacks),
                      [](const std::filesystem::path& videoPackFile) -> VideoPackPtr {
                          return std::make_shared<VideoPack>(videoPackFile);
                      });
    return videoPacks;
}
} // namespace database