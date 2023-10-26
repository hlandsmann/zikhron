#pragma once
#include "VocableMeta.h"
#include "srtypes.h"

#include <annotation/CardDB.h>
#include <annotation/Ease.h>
#include <annotation/Markup.h>
#include <annotation/ZH_Annotator.h>
#include <folly/sorted_vector_types.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <utils/index_map.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include <sys/types.h>
namespace sr {

struct CardMeta
{
    CardMeta(std::shared_ptr<Card> card,
             std::shared_ptr<utl::index_map<VocableId, VocableMeta>> vocables);
    [[nodiscard]] auto Id() const -> CardId;
    [[nodiscard]] auto VocableIndices() const -> const index_set&;
    [[nodiscard]] auto VocableIds() const -> const folly::sorted_vector_set<VocableId>&;
    [[nodiscard]] auto getTimingAndVocables(bool pull = false) const -> const TimingAndVocables&;
    void resetTimingAndVocables();

    [[nodiscard]] auto getStudyMarkup() -> std::unique_ptr<markup::Paragraph>;
    [[nodiscard]] auto getAnnotationMarkup() -> std::unique_ptr<markup::Paragraph>;
    [[nodiscard]] auto getEaseList() -> std::vector<Ease>;

private:
    [[nodiscard]] auto generateTimingAndVocables(bool pull) const -> TimingAndVocables;
    auto generateVocableIDs() const -> const folly::sorted_vector_set<VocableId>&;
    auto generateVocableIndexes() const -> index_set;
    std::shared_ptr<Card> card;
    mutable std::optional<folly::sorted_vector_set<std::size_t>> optVocableIndices;
    mutable std::optional<folly::sorted_vector_set<VocableId>> optVocableIds;
    mutable std::optional<TimingAndVocables> timingAndVocables;
    mutable std::optional<TimingAndVocables> timingAndVocablesPulled;
    std::shared_ptr<utl::index_map<VocableId, VocableMeta>> vocables;
};
} // namespace sr
