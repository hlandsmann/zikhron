#pragma once
#include <cstddef>
namespace sr{

struct Path
{
    size_t cardIndex{};
    size_t eliminateCount{};
    size_t maxVocableSize{};
    size_t steps{};
};
}
