#pragma once
#include "srtypes.h"

#include <annotation/Ease.h>
#include <database/SpacedRepetitionData.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <utils/index_map.h>

#include <cstddef>
#include <memory>
#include <set>

namespace sr {

class VocableMeta
{
public:
    VocableMeta( std::shared_ptr<database::SpacedRepetitionData> spacedRepetitionData);
    [[nodiscard]] auto SpacedRepetitionData() const -> std::shared_ptr<database::SpacedRepetitionData>;
    [[nodiscard]] auto CardIds() const -> const std::set<CardId>&;
    void triggerByCardId(CardId cardId);
    [[nodiscard]] auto triggerValue(CardId cardId) const -> std::size_t;
    void advanceByEase(const Ease&);
    void triggerByCardId(CardId cardId, const utl::index_map<CardId, CardMeta>& cards);
    [[nodiscard]] auto getNextTriggerCard() const -> CardId;

    void insertCardId(CardId);
    void eraseCardId(CardId);
    void setEnabled(bool enabled);

private:
    std::shared_ptr<database::SpacedRepetitionData> spacedRepetitionData;
    std::set<CardId> cardIds;
};

} // namespace sr
