#pragma once
#include <misc/Identifier.h>
// #include <folly/sorted_vector_types.h>
#include <map>
#include <set>

#include <cstddef>

namespace sr {
// using index_set = folly::sorted_vector_set<std::size_t>;
// using vocId_set = folly::sorted_vector_set<VocableId>;
// using cardId_set = folly::sorted_vector_set<CardId>;
// using vocId_vocId_map = folly::sorted_vector_map<VocableId, VocableId>;
using index_set = std::set<std::size_t>;
using vocId_set = std::set<VocableId>;
using cardId_set = std::set<CardId>;
using vocId_vocId_map = std::map<VocableId, VocableId>;

class VocableMeta;
class DataBase;
class CardMeta;
struct TimingAndVocables
{
    int timing{};
    index_set vocables;
};
} // namespace sr
