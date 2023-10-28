#pragma once
#include <map>
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
             std::shared_ptr<utl::index_map<VocableId, VocableMeta>> vocables,
             vocId_vocId_map vocableChoices);
    [[nodiscard]] auto Id() const -> CardId;
    [[nodiscard]] auto VocableIndices() const -> const index_set&;
    [[nodiscard]] auto VocableIds() const -> const folly::sorted_vector_set<VocableId>&;
    [[nodiscard]] auto getTimingAndVocables(bool pull = false) const -> const TimingAndVocables&;
    void resetTimingAndVocables();
    void addVocableChoice(VocableId oldVocId, VocableId newVocId);

    [[nodiscard]] auto getStudyMarkup() -> std::unique_ptr<markup::Paragraph>;
    [[nodiscard]] auto getAnnotationMarkup() -> std::unique_ptr<markup::Paragraph>;
    [[nodiscard]] auto getRelevantEase()const -> std::map<VocableId, Ease>;

private:
    [[nodiscard]] auto generateTimingAndVocables(bool pull) const -> TimingAndVocables;
    auto generateVocableIDs() const -> std::vector<VocableId>;
    auto generateVocableIndexes() const -> index_set;
    void mapVocableChoices(std::vector<VocableId> &vocableIds) const;
    auto getActiveVocableIds() const -> std::vector<VocableId>;
    auto easesFromVocableIds(const std::vector<VocableId>& vocableIds) const -> std::vector<Ease>;
    std::shared_ptr<Card> card;
    mutable std::optional<folly::sorted_vector_set<std::size_t>> optVocableIndices;
    mutable std::optional<folly::sorted_vector_set<VocableId>> optVocableIds;
    mutable std::optional<TimingAndVocables> timingAndVocables;
    mutable std::optional<TimingAndVocables> timingAndVocablesPulled;
    std::shared_ptr<utl::index_map<VocableId, VocableMeta>> vocables;
    vocId_vocId_map vocableChoices;
};
} // namespace sr
