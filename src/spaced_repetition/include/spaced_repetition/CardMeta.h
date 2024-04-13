#pragma once
#include "VocableMeta.h"
#include "srtypes.h"

#include <annotation/Card.h>
#include <annotation/CardDB.h>
#include <annotation/Ease.h>
#include <markup/Markup.h>
#include <annotation/TokenText.h>
#include <annotation/ZH_Tokenizer.h>
#include <dictionary/ZH_Dictionary.h>
#include <folly/sorted_vector_types.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <utils/index_map.h>

#include <map>
#include <memory>
#include <optional>
#include <vector>

#include <sys/types.h>

namespace sr {

struct CardMeta
{
    CardMeta() = default;
    CardMeta(std::shared_ptr<Card> card,
             std::shared_ptr<utl::index_map<VocableId, VocableMeta>> vocables,
             vocId_vocId_map vocableChoices);
    [[nodiscard]] auto Id() const -> CardId;
    [[nodiscard]] auto VocableIndices() const -> const index_set&;
    [[nodiscard]] auto VocableIds() const -> const vocId_set&;
    [[nodiscard]] auto NewVocableIds() const -> vocId_set;
    [[nodiscard]] auto getTimingAndVocables(bool pull = false) const -> const TimingAndVocables&;
    void resetTimingAndVocables();
    void resetAnnotation();
    void addVocableChoice(VocableId oldVocId, VocableId newVocId);

    [[nodiscard]] auto getStudyMarkup() -> std::unique_ptr<markup::Paragraph>;
    [[nodiscard]] auto getStudyTokenText() -> std::unique_ptr<annotation::TokenText>;

    [[nodiscard]] auto getAnnotationMarkup() -> std::unique_ptr<markup::Paragraph>;
    [[nodiscard]] auto getRelevantEase() const -> std::map<VocableId, Ease>;
    [[nodiscard]] auto getDictionary() const -> std::shared_ptr<const ZH_Dictionary>;

private:
    [[nodiscard]] auto generateTimingAndVocables(bool pull) const -> TimingAndVocables;
    auto generateVocableIDs() const -> std::vector<VocableId>;
    auto generateVocableIndexes() const -> index_set;
    void mapVocableChoices(std::vector<VocableId>& vocableIds) const;
    auto getActiveVocableIds() const -> std::vector<VocableId>;
    auto easesFromVocableIds(const std::vector<VocableId>& vocableIds) const -> std::vector<Ease>;
    std::shared_ptr<Card> card;
    mutable std::optional<index_set> optVocableIndices;
    mutable std::optional<vocId_set> optVocableIds;
    mutable std::optional<TimingAndVocables> timingAndVocables;
    mutable std::optional<TimingAndVocables> timingAndVocablesPulled;
    std::shared_ptr<utl::index_map<VocableId, VocableMeta>> vocables;
    vocId_vocId_map vocableChoices;
};
} // namespace sr
