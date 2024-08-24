#pragma once
#include "Card.h"
#include "CbdFwd.h"
#include "IdGenerator.h"
#include "Subtitle.h"
#include "WordDB.h"

#include <annotation/Tokenizer.h>
#include <misc/Identifier.h>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
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
    static constexpr std::string_view s_progressType = "subpickprgs";
    static constexpr std::string_view s_progressExtension = "zpgs";

public:
    SubtitlePicker(std::shared_ptr<Subtitle> subtitle,
                   PackId videoId,
                   std::string videoName,
                   const std::shared_ptr<CardIdGenerator>& cardIdGenerator,
                   std::shared_ptr<annotation::Tokenizer> tokenizer,
                   std::shared_ptr<WordDB> wordDB);

    auto getDeserializedActiveCards() -> std::vector<CardPtr>;

    [[nodiscard]] auto isFrontJoinable(const CardPtr& card) const -> bool;
    [[nodiscard]] auto isBackJoinable(const CardPtr& card) const -> bool;
    [[nodiscard]] auto isSeparable(const CardPtr& card) const -> bool;

    [[nodiscard]] auto joinFront(const CardPtr& card) -> JoinedSubtitle;
    [[nodiscard]] auto joinBack(const CardPtr& card) -> JoinedSubtitle;
    [[nodiscard]] auto cutFront(const CardPtr& card) -> JoinedSubtitle;
    [[nodiscard]] auto cutBack(const CardPtr& card) -> JoinedSubtitle;

    [[nodiscard]] auto getPrevious(const CardPtr& card) -> JoinedSubtitle;
    [[nodiscard]] auto getNext(const CardPtr& card) -> JoinedSubtitle;
    [[nodiscard]] auto static hasPrevious(const CardPtr& card) -> bool;
    [[nodiscard]] auto hasNext(const CardPtr& card) const -> bool;

    [[nodiscard]] auto numberOfCards() -> std::size_t;
    [[nodiscard]] auto getJoinedSubAtPosition(std::size_t pos) -> JoinedSubtitle;
    [[nodiscard]] auto getJoinedSubAtIndex(std::size_t index) -> JoinedSubtitle;
    [[nodiscard]] auto joinedSubtitleFromCard(const CardPtr& card) -> JoinedSubtitle;
    [[nodiscard]] auto joinedSubtitleFromLastActiveCard() -> JoinedSubtitle;

    void timeAddBack(const CardPtr& card);
    void timeAddFront(const CardPtr& card);
    void timeDelBack(const CardPtr& card);
    void timeDelFront(const CardPtr& card);
    [[nodiscard]] auto getTimeExtraBack(const CardPtr& card) -> double;
    [[nodiscard]] auto getTimeExtraFront(const CardPtr& card) -> double;
    void save();

private:
    void deserialize();
    [[nodiscard]] auto serialize() const -> std::string;
    [[nodiscard]] auto createJoinedSubtitle(std::size_t index, const CardPtr& card) -> JoinedSubtitle;
    void setIndices();
    void removeCardAtIndex(std::size_t index);
    [[nodiscard]] static auto genCardIds(const std::shared_ptr<CardIdGenerator>& cardIdGenerator,
                                         std::size_t size) -> std::vector<CardId>;
    std::shared_ptr<Subtitle> subtitle;
    std::filesystem::path progressFile;
    PackId videoId;
    std::string videoName;
    std::shared_ptr<annotation::Tokenizer> tokenizer;
    std::shared_ptr<WordDB> wordDB;

    std::vector<std::size_t> joinings;
    std::vector<std::size_t> indices;
    std::vector<std::weak_ptr<SubtitleCard>> cards;
    std::vector<CardId> cardIds;

    std::vector<double> timeExtraBack;
    std::vector<double> timeExtraFront;

    std::vector<std::size_t> deserializedActiveCardIndices;
};

using SubtitlePickerPtr = std::shared_ptr<SubtitlePicker>;

} // namespace database
