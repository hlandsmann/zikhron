#pragma once
#include "Video.h"

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace database {

class VideoSet
{
    static constexpr std::string_view s_type = "videoSet";

public:
    VideoSet(std::filesystem::path videoSetFile);
    VideoSet(std::filesystem::path videoFile,
              std::string name,
              const std::vector<std::filesystem::path>& videoFiles);
    [[nodiscard]] auto getName() const -> const std::string&;
    [[nodiscard]] auto getVideo() const -> VideoPtr;
    void save();

private:
    void deserialize();
    [[nodiscard]] auto serialize() const -> std::string;
    static auto genVideosFromPaths(const std::vector<std::filesystem::path>& videoFiles,
                                   const std::filesystem::path& videoSetFile) -> std::vector<VideoPtr>;
    std::filesystem::path videoSetFile;
    std::string name;

    std::vector<VideoPtr> videos;
};

using VideoSetPtr = std::shared_ptr<VideoSet>;

} // namespace database
