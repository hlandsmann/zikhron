#pragma once
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace database {

class Video
{
public:
    Video(std::string_view sv);
    Video(std::filesystem::path videoFile);
    [[nodiscard]] auto serialize() const -> std::string;

private:
    void deserialize(std::string_view content);
    std::filesystem::path videoFile;
};

using VideoPtr = std::shared_ptr<Video>;
} // namespace database
