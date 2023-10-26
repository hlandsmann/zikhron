#pragma once
#include <folly/sorted_vector_types.h>

#include <cstddef>

namespace sr {
using index_set = folly::sorted_vector_set<std::size_t>;
class VocableMeta;
class DataBase;
struct CardMeta;
struct TimingAndVocables
{
    int timing{};
    index_set vocables{};
};
} // namespace sr
