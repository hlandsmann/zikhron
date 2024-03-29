#pragma once
#include "DataBase.h"
#include "VocableProgress.h"
#include "srtypes.h"

#include <annotation/CardDB.h>
#include <annotation/Ease.h>
#include <annotation/ZH_Tokenizer.h>
#include <folly/sorted_vector_types.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <utils/index_map.h>

#include <cstddef>
#include <memory>

namespace sr {

class VocableMeta
{
public:
    VocableMeta(VocableProgress progress);
    [[nodiscard]] auto Progress() const -> const VocableProgress&;
    [[nodiscard]] auto CardIndices() const -> const folly::sorted_vector_set<std::size_t>&;
    void advanceByEase(const Ease&);
    void triggerByCardId(CardId cardId);
    [[nodiscard]] auto getNextTriggerCard(const std::shared_ptr<DataBase>& db) const -> CardId;

    void cardIndices_insert(std::size_t cardIndex);

private:
    VocableProgress progress;
    folly::sorted_vector_set<std::size_t> cardIndices;
    // ZH_Tokenizer::ZH_dicItemVec dicItemVec;
};

} // namespace sr
