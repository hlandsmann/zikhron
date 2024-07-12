#pragma once
#include <string_view>
#include "Video.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace database {

class VideoPack
{
    static constexpr std::string_view s_type = "videoPack";

public:
    VideoPack(std::filesystem::path videoPackFile);
    VideoPack(std::filesystem::path videoPackFile,
              std::string name,
              const std::vector<std::filesystem::path>& videoFiles);
    [[nodiscard]] auto getName() const -> const std::string&;
    void save();

private:
    void deserialize();
    [[nodiscard]] auto serialize() const -> std::string;
    static auto genVideosFromPaths(const std::vector<std::filesystem::path>& videoFiles) -> std::vector<VideoPtr>;
    std::filesystem::path videoPackFile;
    std::string name;

    std::vector<VideoPtr> videos;
};

using VideoPackPtr = std::shared_ptr<VideoPack>;

} // namespace database
