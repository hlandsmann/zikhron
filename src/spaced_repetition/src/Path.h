#pragma once
#include <misc/Identifier.h>
#include <cstddef>
namespace sr{

struct Path
{
    CardId cardId{};
    size_t eliminateCount{};
    size_t maxVocableSize{};
    size_t steps{};
};
}
