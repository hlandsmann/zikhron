#include <DataBase.h>
#include <VocableMeta.h>
#include <annotation/Ease.h>
#include <database/VocableProgress.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <srtypes.h>
#include <utils/index_map.h>
#include <utils/min_element_val.h>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include <sys/types.h>

namespace ranges = std::ranges;

// namespace views = std::views;

namespace sr {
VocableMeta::VocableMeta(std::shared_ptr<VocableProgress> _progress)
    : progress{std::move(_progress)}
{}

auto VocableMeta::Progress() const -> const VocableProgress&
{
    return *progress;
}

auto VocableMeta::CardIds() const -> const std::set<CardId>&
{
    return cardIds;
}

void VocableMeta::advanceByEase(const Ease& ease)
{
    progress->advanceByEase(ease);
}

void VocableMeta::triggerByCardId(CardId cardId)
{
    std::vector<CardId> cardIdVec;
    ranges::copy(cardIds, std::back_inserter(cardIdVec));
    progress->triggeredBy(cardId, cardIdVec);
}

auto VocableMeta::triggerValue(CardId cardId) const -> std::size_t
{
    std::vector<CardId> cardIdVec;
    ranges::copy(cardIds, std::back_inserter(cardIdVec));
    return progress->triggerValue(cardId, cardIdVec);
}

auto VocableMeta::getNextTriggerCard() const -> CardId
{
    std::vector<CardId> cardIdVec;
    ranges::copy(cardIds, std::back_inserter(cardIdVec));
    return progress->getNextTriggerCard(cardIdVec);
}

void VocableMeta::insertCardId(CardId cardId)
{
    cardIds.insert(cardId);
}

void VocableMeta::eraseCardId(CardId cardId)
{
    cardIds.erase(cardId);
}

} // namespace sr
