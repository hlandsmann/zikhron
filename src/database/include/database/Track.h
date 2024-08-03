#pragma once
#include "CardPack.h"
#include "CbdFwd.h"
#include "Video.h"

#include <misc/Identifier.h>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
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
    Track(TrackMedia medium, std::size_t index);
    Track(TrackMedia medium, CardPtr card);

    [[nodiscard]] auto numberOfTracks() const -> std::size_t;
    [[nodiscard]] auto trackAt(std::size_t index) const -> Track;
    [[nodiscard]] auto nextTrack() const -> std::optional<Track>;
    [[nodiscard]] auto previousTrack() const -> std::optional<Track>;

    [[nodiscard]] auto getCardID() const -> std::optional<CardId>;
    [[nodiscard]] auto getTrackType() const -> TrackType;

    [[nodiscard]] auto getMediaFile() const -> std::optional<std::filesystem::path>;
    [[nodiscard]] auto getStartTimeStamp() const -> double;
    [[nodiscard]] auto getEndTimeStamp() const -> double;

private:
    void setupTimeStamps();
    TrackMedia medium;
    std::size_t index;
    double startTimeStamp{};
    double endTimeStamp{};
};
} // namespace database
