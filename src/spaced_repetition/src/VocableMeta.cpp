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
#include <memory>
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

auto VocableMeta::CardIndices() const -> const index_set&
{
    return cardIndices;
}

void VocableMeta::advanceByEase(const Ease& ease)
{
    progress->advanceByEase(ease);
}

void VocableMeta::triggerByCardId(CardId cardId, const utl::index_map<CardId, CardMeta>& cards)
{
    auto cardIds = getCardIds(cards);
    progress->triggeredBy(cardId, cardIds);
}

auto VocableMeta::getNextTriggerCard(const utl::index_map<CardId, CardMeta>& cards) const -> CardId
{
    auto cardIds = getCardIds(cards);
    return progress->getNextTriggerCard(cardIds);
}

void VocableMeta::cardIndices_insert(std::size_t cardIndex)
{
    cardIndices.insert(cardIndex);
}

void VocableMeta::cardIndices_erase(std::size_t cardIndex)
{
    cardIndices.erase(cardIndex);
}

auto VocableMeta::getCardIds(const utl::index_map<CardId, CardMeta>& cards) const -> std::vector<CardId>
{
    std::vector<CardId> cardIds;
    ranges::transform(cardIndices, std::inserter(cardIds, cardIds.begin()), [&](size_t cardIndex) -> CardId {
        return cards.id_from_index(cardIndex);
    });
    return cardIds;
}
} // namespace sr
