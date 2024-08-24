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
#include <string>
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
    auto operator==(const Track&) const -> bool = default;
    auto operator!=(const Track&) const -> bool = default;

    [[nodiscard]] auto numberOfTracks() const -> std::size_t;
    [[nodiscard]] auto trackAt(std::size_t index) const -> Track;
    [[nodiscard]] auto continueTrack() const -> Track;
    [[nodiscard]] auto nextTrack() const -> Track;
    [[nodiscard]] auto previousTrack() const -> Track;
    [[nodiscard]] auto hasNext() const -> bool;
    [[nodiscard]] auto hasPrevious() const -> bool;

    [[nodiscard]] auto isFrontJoinable() const -> bool;
    [[nodiscard]] auto isBackJoinable() const -> bool;
    [[nodiscard]] auto isSeparable() const -> bool;

    [[nodiscard]] auto joinFront() const -> Track;
    [[nodiscard]] auto joinBack() const -> Track;
    [[nodiscard]] auto cutFront() const -> Track;
    [[nodiscard]] auto cutBack() const -> Track;

    [[nodiscard]] auto getCard() const -> CardPtr;
    [[nodiscard]] auto getTrackType() const -> TrackType;

    [[nodiscard]] auto getMediaFile() const -> std::optional<std::filesystem::path>;
    [[nodiscard]] auto getStartTimeStamp() const -> double;
    [[nodiscard]] auto getEndTimeStamp() const -> double;

    [[nodiscard]] auto hasSubtitlePrefix() const -> bool;
    [[nodiscard]] auto getSubtitlePrefix() const -> Track;
    [[nodiscard]] auto getNonPrefixDefault() const -> Track;
    [[nodiscard]] auto isSubtitlePrefix() const -> bool;

    [[nodiscard]] auto getTranslation() const -> std::optional<std::string>;

    [[nodiscard]] auto timeAddBack() const -> Track;
    [[nodiscard]] auto timeAddFront() const -> Track;
    [[nodiscard]] auto timeDelBack() const -> Track;
    [[nodiscard]] auto timeDelFront() const -> Track;
    [[nodiscard]] auto getTimeExtraBack() const -> double;
    [[nodiscard]] auto getTimeExtraFront() const -> double;

private:
    Track(TrackMedia medium, const JoinedSubtitle& joinedSubtitle);

    // Prefix ctor
    Track(TrackMedia medium, CardPtr card, double startTimeStamp, double endTimeStamp);

    static auto getCard(TrackMedia medium, std::size_t index) -> CardPtr;
    void setupMedium();
    void setupCardAudio(const CardAudio& cardAudio);
    void setupJoinedSubtitle(const JoinedSubtitle& joinedSubtitle);
    [[nodiscard]] auto getSubtitlePrefixTime() const -> std::pair<double, double>;
    TrackMedia medium;
    CardPtr card;
    double startTimeStamp{};
    double endTimeStamp{};

    bool isPrefixToSubtitle{false};
};
} // namespace database
