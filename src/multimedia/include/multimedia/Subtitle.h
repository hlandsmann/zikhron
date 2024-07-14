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
};

struct Subtitle
{
    std::string language;
    std::string title;
    int indexInVideo;
    std::vector<SubText> subs;
};
} // namespace multimedia
