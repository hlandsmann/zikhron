#pragma once
#include "VocableMeta.h"
#include "srtypes.h"

#include <annotation/Card.h>
#include <annotation/Ease.h>
#include <annotation/ZH_Annotator.h>
#include <folly/sorted_vector_types.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <utils/index_map.h>

#include <cstddef>
#include <functional>
#include <optional>

#include <sys/types.h>
namespace sr {

struct CardMeta
{
    CardMeta(folly::sorted_vector_set<std::size_t> _vocableIndices,
             std::reference_wrapper<utl::index_map<VocableId, VocableMeta>> _refVocables);
    [[nodiscard]] auto VocableIndices() const -> const folly::sorted_vector_set<std::size_t>&;
    void vocableIndices_insert(std::size_t vocableIndex);
    [[nodiscard]] auto getTimingAndVocables(bool pull = false) const -> const TimingAndVocables&;
    void resetTimingAndVocables();

private:
    [[nodiscard]] auto generateTimingAndVocables(bool pull) const -> TimingAndVocables;
    folly::sorted_vector_set<std::size_t> vocableIndices;
    mutable std::optional<TimingAndVocables> timingAndVocables;
    mutable std::optional<TimingAndVocables> timingAndVocablesPulled;
    std::reference_wrapper<utl::index_map<VocableId, VocableMeta>> refVocables;
};
} // namespace sr
