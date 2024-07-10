#include "VideoPackDB.h"

#include <VideoPack.h>
#include <misc/Config.h>
#include <spdlog/spdlog.h>
#include <utils/format.h>

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <memory>
#include <vector>

namespace ranges = std::ranges;

namespace database {

VideoPackDB::VideoPackDB(std::shared_ptr<zikhron::Config> config)
    : videoPackDir{config->DatabaseDirectory() / s_videoSubdirectory}

{
}

auto VideoPackDB::addVideoPack(const std::vector<std::filesystem::path>& videoFiles) -> VideoPackPtr
{
    if (videoFiles.empty() || !std::filesystem::exists(videoFiles.front())) {
        return nullptr;
    }
    const auto& firstVideoFile = videoFiles.front();
    const auto& parentPath = firstVideoFile.parent_path();
    const auto& packName = parentPath.empty()
                                   ? firstVideoFile.stem()
                                   : *std::prev(parentPath.end());
    auto packFilename = videoPackDir / std::filesystem::path{packName.string() + s_videoPackExtension};
    auto videoPack = std::make_shared<VideoPack>(packFilename, packName, videoFiles);
    videoPacks.push_back(videoPack);

    return videoPack;
}

void VideoPackDB::save()
{
    for (const auto& videoPack : videoPacks) {
        videoPack->save();
    }
}
} // namespace database
