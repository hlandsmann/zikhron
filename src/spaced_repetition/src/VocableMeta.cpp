#include <DataBase.h>
#include <VocableMeta.h>
#include <annotation/Ease.h>
#include <database/SpacedRepetitionData.h>
#include <dictionary/DictionaryChi.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <srtypes.h>
#include <utils/index_map.h>
#include <utils/min_element_val.h>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include <sys/types.h>

namespace ranges = std::ranges;

// namespace views = std::views;

namespace sr {
VocableMeta::VocableMeta(std::shared_ptr<database::SpacedRepetitionData> _spacedRepetitionData)
    : spacedRepetitionData{std::move(_spacedRepetitionData)}
{}

auto VocableMeta::SpacedRepetitionData() const -> std::shared_ptr<database::SpacedRepetitionData>
{
    return spacedRepetitionData;
}

auto VocableMeta::CardIds() const -> const std::set<CardId>&
{
    return cardIds;
}

void VocableMeta::triggerByCardId(CardId cardId)
{
    std::vector<CardId> cardIdVec;
    ranges::copy(cardIds, std::back_inserter(cardIdVec));
    spacedRepetitionData->triggeredBy(cardId, cardIdVec);
}

auto VocableMeta::triggerValue(CardId cardId) const -> std::size_t
{
    std::vector<CardId> cardIdVec;
    ranges::copy(cardIds, std::back_inserter(cardIdVec));
    return spacedRepetitionData->triggerValue(cardId, cardIdVec);
}

auto VocableMeta::getNextTriggerCard() const -> CardId
{
    std::vector<CardId> cardIdVec;
    ranges::copy(cardIds, std::back_inserter(cardIdVec));
    return spacedRepetitionData->getNextTriggerCard(cardIdVec);
}

void VocableMeta::insertCardId(CardId cardId)
{
    cardIds.insert(cardId);
}

void VocableMeta::eraseCardId(CardId cardId)
{
    cardIds.erase(cardId);
}

void VocableMeta::setEnabled(bool enabled)
{
    spacedRepetitionData->enabled = enabled;
}

} // namespace sr
