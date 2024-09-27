#pragma once
#include "srtypes.h"

#include <annotation/Ease.h>
#include <database/VocableProgress.h>
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
    VocableMeta(std::shared_ptr<VocableProgress> progress);
    [[nodiscard]] auto Progress() const -> const VocableProgress&;
    [[nodiscard]] auto CardIds() const -> const std::set<CardId>&;
    void triggerByCardId(CardId cardId);
    [[nodiscard]] auto triggerValue(CardId cardId) const -> std::size_t;
    void advanceByEase(const Ease&);
    void triggerByCardId(CardId cardId, const utl::index_map<CardId, CardMeta>& cards);
    [[nodiscard]] auto getNextTriggerCard() const -> CardId;

    void insertCardId(CardId);
    void eraseCardId(CardId);

private:
    std::shared_ptr<VocableProgress> progress;
    std::set<CardId> cardIds;
};

} // namespace sr
