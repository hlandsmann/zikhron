#pragma once
#include "Subtitle.h"

#include <multimedia/Subtitle.h>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace database {

class Video
{
public:
    Video(std::string_view sv, std::filesystem::path videoPackFile);
    Video(std::filesystem::path videoFile, std::filesystem::path videoPackFile);
    [[nodiscard]] auto serialize() const -> std::string;
    void loadSubtitles();

private:
    void deserialize(std::string_view content);
    std::filesystem::path videoPackFile;
    std::filesystem::path videoFile;
    std::string name;

    std::vector<SubtitlePtr> subtitles;
    std::size_t subChoice = 0;
};

using VideoPtr = std::shared_ptr<Video>;
} // namespace database
