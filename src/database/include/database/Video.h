#pragma once
#include <multimedia/ExtractSubtitles.h>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace multimedia {
class ExtractSubtitles;
}

namespace database {

class Video
{
public:
    Video(std::string_view sv);
    Video(std::filesystem::path videoFile);
    [[nodiscard]] auto serialize() const -> std::string;
    void loadSubtitles();

private:
    void deserialize(std::string_view content);
    std::filesystem::path videoFile;

    std::unique_ptr<multimedia::ExtractSubtitles> subtitleDecoder;
};

using VideoPtr = std::shared_ptr<Video>;
} // namespace database
