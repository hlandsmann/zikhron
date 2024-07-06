#pragma once
#include "DataBase.h"
#include "srtypes.h"

#include <annotation/Ease.h>
#include <database/VocableProgress.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <utils/index_map.h>

#include <cstddef>
#include <memory>
#include <vector>

namespace sr {

class VocableMeta
{
public:
    VocableMeta(std::shared_ptr<VocableProgress> progress);
    [[nodiscard]] auto Progress() const -> const VocableProgress&;
    [[nodiscard]] auto CardIndices() const -> const index_set&;
    void advanceByEase(const Ease&);
    void triggerByCardId(CardId cardId, const utl::index_map<CardId, CardMeta>& cards);
    [[nodiscard]] auto getNextTriggerCard(const utl::index_map<CardId, CardMeta>& cards) const -> CardId;

    void cardIndices_insert(std::size_t cardIndex);
    void cardIndices_erase(std::size_t cardIndex);

private:
    [[nodiscard]] auto getCardIds(const utl::index_map<CardId, CardMeta>& cards) const -> std::vector<CardId>;
    std::shared_ptr<VocableProgress> progress;
    index_set cardIndices;
};

} // namespace sr
