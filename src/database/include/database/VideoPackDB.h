#pragma once
#include "VideoPack.h"

#include <misc/Config.h>

#include <filesystem>
#include <memory>
#include <vector>

namespace database {

class VideoPackDB
{
    static constexpr auto s_videoSubdirectory = "video";
    static constexpr auto s_videoPackExtension = ".vpkg";

public:
    VideoPackDB(std::shared_ptr<zikhron::Config> config);
    auto addVideoPack(const std::vector<std::filesystem::path>& videoFiles) -> VideoPackPtr;
    void save();

private:
    std::vector<VideoPackPtr> videoPacks;

    std::filesystem::path videoPackDir;
};

} // namespace database
