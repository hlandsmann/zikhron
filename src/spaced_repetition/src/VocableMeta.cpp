#include <DataBase.h>
#include <VocableMeta.h>
#include <VocableProgress.h>
#include <annotation/CardDB.h>
#include <annotation/Ease.h>
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
#include <memory>
#include <set>
#include <utility>

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

auto VocableMeta::CardIndices() const -> const index_set&
{
    return cardIndices;
}

void VocableMeta::advanceByEase(const Ease& ease)
{
    progress->advanceByEase(ease);
}

void VocableMeta::triggerByCardId(CardId cardId)
{
    progress->triggeredBy(cardId);
}

auto VocableMeta::getNextTriggerCard(const std::shared_ptr<DataBase>& db) const -> CardId
{
    std::set<CardId> cardIds;
    ranges::transform(cardIndices, std::inserter(cardIds, cardIds.begin()), [&](size_t cardIndex) -> CardId {
        return db->Cards().id_from_index(cardIndex);
    });
    return progress->getNextTriggerCard(std::move(cardIds));
}

void VocableMeta::cardIndices_insert(std::size_t cardIndex)
{
    cardIndices.insert(cardIndex);
}
} // namespace sr
