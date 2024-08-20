#pragma once
#include "CardPack.h"
#include "CbdFwd.h"
#include "SubtitlePicker.h"
#include "Video.h"

#include <misc/Identifier.h>
#include <multimedia/Subtitle.h>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <utility>
#include <variant>

namespace database {
enum class TrackType {
    audio,
    video,
    noMedia,
};

using TrackMedia = std::variant<CardPackPtr,
                                VideoPtr>;

class Track
{
public:
    Track(TrackMedia medium, CardPtr card);
    Track(const TrackMedia& medium, std::size_t index);

    [[nodiscard]] auto numberOfTracks() const -> std::size_t;
    [[nodiscard]] auto trackAt(std::size_t index) const -> Track;
    [[nodiscard]] auto nextTrack() const -> Track;
    [[nodiscard]] auto previousTrack() const -> Track;
    [[nodiscard]] auto hasNext() const -> bool;
    [[nodiscard]] auto hasPrevious() const -> bool;

    [[nodiscard]] auto isFrontJoinable() const -> bool;
    [[nodiscard]] auto isBackJoinable() const -> bool;
    [[nodiscard]] auto isSeparable() const -> bool;

    [[nodiscard]] auto joinFront()const -> Track;
    [[nodiscard]] auto joinBack()const -> Track;
    [[nodiscard]] auto cutFront()const -> Track;
    [[nodiscard]] auto cutBack()const -> Track;

    [[nodiscard]] auto getCard() const -> CardPtr;
    [[nodiscard]] auto getTrackType() const -> TrackType;

    [[nodiscard]] auto getMediaFile() const -> std::optional<std::filesystem::path>;
    [[nodiscard]] auto getStartTimeStamp() const -> double;
    [[nodiscard]] auto getEndTimeStamp() const -> double;

    [[nodiscard]] auto getSubtitlePrefix() -> Track;
    [[nodiscard]] auto getNonPrefixDefault() -> Track;
    [[nodiscard]] auto isSubtitlePrefix() const -> bool;

private:
    Track(TrackMedia medium, const JoinedSubtitle& joinedSubtitle);

    // Prefix ctor
    Track(TrackMedia medium,  CardPtr card, double startTimeStamp, double endTimeStamp);

    static auto getCard(TrackMedia medium, std::size_t index) -> CardPtr;
    void setupMedium();
    void setupCardAudio(const CardAudio& cardAudio);
    void setupJoinedSubtitle(const JoinedSubtitle& joinedSubtitle);
    TrackMedia medium;
    CardPtr card;
    double startTimeStamp{};
    double endTimeStamp{};

    bool isPrefixToSubtitle{false};
};
} // namespace database
