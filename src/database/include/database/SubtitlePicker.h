#pragma once
#include "Card.h"
#include "CbdFwd.h"
#include "IdGenerator.h"
#include "Subtitle.h"

#include <misc/Identifier.h>

#include <cstddef>
#include <memory>
#include <vector>

namespace database {
struct JoinedSubtitle
{
    std::shared_ptr<SubtitleCard> card;
    double startTimeStamp;
    double endTimeStamp;
};

class SubtitlePicker
{
public:
    SubtitlePicker(std::shared_ptr<Subtitle> subtitle,
                   PackId videoId,
                   std::shared_ptr<CardIdGenerator> cardIdGenerator);
    auto getActiveCards() -> std::vector<CardPtr>;

    [[nodiscard]] auto isFrontJoinable(const CardPtr& card) const -> bool;
    [[nodiscard]] auto isBackJoinable(const CardPtr& card) const -> bool;
    [[nodiscard]] auto isFrontSeparable(const CardPtr& card) const -> bool;
    [[nodiscard]] auto isBackSeparable(const CardPtr& card) const -> bool;

    [[nodiscard]] auto joinFront(const CardPtr& card) -> CardPtr;
    [[nodiscard]] auto joinBack(const CardPtr& card) -> CardPtr;

    [[nodiscard]] auto getPrevious(const CardPtr& card) -> CardPtr;
    [[nodiscard]] auto getNext(const CardPtr& card) -> CardPtr;

    void save();

private:
    std::shared_ptr<Subtitle> subtitle;
    PackId videoId;
    std::shared_ptr<CardIdGenerator> cardIdGenerator;

    std::vector<std::size_t> joinings;
    std::vector<std::weak_ptr<SubtitleCard>> cards;

    std::vector<std::size_t> activeCardIndices;
};

} // namespace database
