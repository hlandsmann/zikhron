#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace multimedia {

struct SubText
{
    int64_t startTime;
    int64_t duration;
    std::string style;
    std::string text;

    [[nodiscard]] auto getStartTimeStamp() const -> double
    {
        return static_cast<double>(startTime) / 1000.;
    }

    [[nodiscard]] auto getEndTimeStamp() const -> double
    {
        return static_cast<double>(startTime + duration) / 1000.;
    }
};

struct Subtitle
{
    std::string language;
    std::string title;
    int indexInVideo;
    std::vector<SubText> subs;
};
} // namespace multimedia
