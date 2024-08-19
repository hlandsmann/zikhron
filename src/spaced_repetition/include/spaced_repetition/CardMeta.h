#pragma once
#include "VocableMeta.h"
#include "srtypes.h"

#include <annotation/Ease.h>
#include <annotation/TokenText.h>
#include <database/Card.h>
#include <database/CbdFwd.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <utils/index_map.h>

#include <map>
#include <memory>
#include <optional>
#include <vector>

#include <sys/types.h>

namespace sr {

class CardMeta
{
    using Card = database::Card;

public:
    CardMeta() = default;
    CardMeta(CardId cardId,
             std::shared_ptr<Card> card,
             std::shared_ptr<utl::index_map<VocableId, VocableMeta>> vocables);
    [[nodiscard]] auto isValid() const -> bool;
    [[nodiscard]] auto getCardId() const -> CardId;
    [[nodiscard]] auto getCard() const -> database::CardPtr;
    [[nodiscard]] auto VocableIndices() const -> const index_set&;
    [[nodiscard]] auto VocableIds() const -> const vocId_set&;
    [[nodiscard]] auto NewVocableIds() const -> vocId_set;
    [[nodiscard]] auto getTimingAndVocables(bool pull = false) const -> const TimingAndVocables&;
    void resetTimingAndVocables();

    [[nodiscard]] auto getStudyTokenText() -> std::unique_ptr<annotation::TokenText>;

    [[nodiscard]] auto getRelevantEase() const -> std::map<VocableId, Ease>;

    void resetMetaData();

private:
    [[nodiscard]] auto generateTimingAndVocables(bool pull) const -> TimingAndVocables;
    auto generateVocableIDs() const -> std::vector<VocableId>;
    auto generateVocableIndexes() const -> index_set;
    auto getActiveVocableIds() const -> std::vector<VocableId>;
    auto easesFromVocableIds(const std::vector<VocableId>& vocableIds) const -> std::vector<Ease>;
    CardId cardId{};
    std::shared_ptr<Card> card;
    mutable std::optional<index_set> optVocableIndices;
    mutable std::optional<vocId_set> optVocableIds;
    mutable std::optional<TimingAndVocables> timingAndVocables;
    mutable std::optional<TimingAndVocables> timingAndVocablesPulled;
    std::shared_ptr<utl::index_map<VocableId, VocableMeta>> vocables;
};
} // namespace sr
