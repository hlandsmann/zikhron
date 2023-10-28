#pragma once
#include <misc/Identifier.h>
#include <folly/sorted_vector_types.h>

#include <cstddef>

namespace sr {
using index_set = folly::sorted_vector_set<std::size_t>;
using vocId_set = folly::sorted_vector_set<VocableId>;
using cardId_set = folly::sorted_vector_set<CardId>;
using vocId_vocId_map = folly::sorted_vector_map<VocableId, VocableId>;

class VocableMeta;
class DataBase;
struct CardMeta;
struct TimingAndVocables
{
    int timing{};
    index_set vocables{};
};
} // namespace sr
