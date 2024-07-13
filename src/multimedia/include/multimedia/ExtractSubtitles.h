#pragma once
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <stop_token>
#include <string>
#include <vector>

namespace multimedia {
struct SubText
{
    std::string style;
    std::string text;
    int64_t startTime;
    int64_t duration;
};

struct Subtitle
{
    std::string language;
    std::string title;
    int indexInVideo;
    std::vector<SubText> subs;
};

class ExtractSubtitles
{
public:
    ExtractSubtitles(std::filesystem::path videoFile);
    auto decode(std::stop_token token) -> std::vector<Subtitle>;
    [[nodiscard]] auto getProgress() const -> double;

private:
    std::string filename;
    std::atomic<double> progress = 0.;
};
} // namespace multimedia
