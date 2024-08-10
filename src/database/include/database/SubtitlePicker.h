#pragma once
#include "Card.h"
#include "CbdFwd.h"
#include "IdGenerator.h"
#include "Subtitle.h"
#include "WordDB.h"

#include <annotation/Tokenizer.h>
#include <misc/Identifier.h>

#include <cstddef>
#include <memory>
#include <string>
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
                   std::string videoName,
                   std::shared_ptr<CardIdGenerator> cardIdGenerator,
                   std::shared_ptr<annotation::Tokenizer> tokenizer,
                   std::shared_ptr<WordDB> wordDB);
    auto getActiveCards() -> std::vector<CardPtr>;

    [[nodiscard]] auto isFrontJoinable(const CardPtr& card) const -> bool;
    [[nodiscard]] auto isBackJoinable(const CardPtr& card) const -> bool;
    [[nodiscard]] auto isSeparable(const CardPtr& card) const -> bool;

    [[nodiscard]] auto joinFront(const CardPtr& card) -> JoinedSubtitle;
    [[nodiscard]] auto joinBack(const CardPtr& card) -> JoinedSubtitle;
    [[nodiscard]] auto cutFront(const CardPtr& card) -> JoinedSubtitle;
    [[nodiscard]] auto cutBack(const CardPtr& card) -> JoinedSubtitle;

    [[nodiscard]] auto getPrevious(const CardPtr& card) -> JoinedSubtitle;
    [[nodiscard]] auto getNext(const CardPtr& card) -> JoinedSubtitle;

    [[nodiscard]] auto numberOfCards() -> std::size_t;
    [[nodiscard]] auto getJoinedSubAt(std::size_t pos) -> JoinedSubtitle;

    void save();

private:
    [[nodiscard]] auto createJoinedSubtitle(std::size_t index, const CardPtr& card) -> JoinedSubtitle;
    [[nodiscard]] auto joinedSubtitleFromCard(const CardPtr& card) -> JoinedSubtitle;
    void setIndices();
    std::shared_ptr<Subtitle> subtitle;
    PackId videoId;
    std::string videoName;
    std::shared_ptr<CardIdGenerator> cardIdGenerator;
    std::shared_ptr<annotation::Tokenizer> tokenizer;
    std::shared_ptr<WordDB> wordDB;

    std::vector<std::size_t> joinings;
    std::vector<std::size_t> indices;
    std::vector<std::weak_ptr<SubtitleCard>> cards;

    // std::vector<std::size_t> activeCardIndices;
};

using SubtitlePickerPtr = std::shared_ptr<SubtitlePicker>;

} // namespace database
