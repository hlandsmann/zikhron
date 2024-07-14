#pragma once
#include "Subtitle.h"

#include <atomic>
#include <filesystem>
#include <stop_token>
#include <string>
#include <vector>

namespace multimedia {

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
