#include "Track.h"

#include "Card.h" // IWYU pragma: keep
#include "CardPack.h"
#include "CbdFwd.h"
#include "Video.h"

#include <misc/Identifier.h>
#include <multimedia/Subtitle.h>
#include <spdlog/spdlog.h>
#include <utils/Variant.h>

#include <cstddef>
#include <exception>
#include <filesystem>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>

namespace database {

Track::Track(TrackMedia _medium, std::size_t _index)
    : medium{std::move(_medium)}
    , index{_index}
{
    try {
        setupTimeStamps();
    } catch (const std::exception& e) {
        spdlog::error("track, e.what: {}", e.what());
    }
}

Track::Track(TrackMedia _medium, CardPtr card)
    : Track{std::move(_medium), card->getIndexInPack()}
{}

auto Track::numberOfTracks() const -> std::size_t
{
    return std::visit(utl::overloaded{[](const CardPackPtr& cardPack) -> std::size_t {
                                          return cardPack->getNumberOfCards();
                                      },
                                      [](const VideoPtr& video) -> std::size_t {
                                          return video->getActiveSubtitle()->numberOfCards();
                                      }},
                      medium);
}

auto Track::trackAt(std::size_t _index) const -> Track
{
    return {medium, _index};
}

auto Track::nextTrack() const -> std::optional<Track>
{
    if (index + 1 < numberOfTracks()) {
        return {{medium, index + 1}};
    }
    return {};
}

auto Track::previousTrack() const -> std::optional<Track>
{
    if (index > 0) {
        return {{medium, index - 1}};
    }
    return {};
}

auto Track::getCardID() const -> std::optional<CardId>
{
    return std::visit(utl::overloaded{[this](const CardPackPtr& cardPack) -> std::optional<CardId> {
                                          return {cardPack->getCardByIndex(index).card->getCardId()};
                                      },
                                      [](const VideoPtr& video) -> std::optional<CardId> {
                                          return {};
                                      }},
                      medium);
}

auto Track::getTrackType() const -> TrackType
{
    if (std::holds_alternative<VideoPtr>(medium)) {
        return TrackType::video;
    }
    if (std::holds_alternative<CardPackPtr>(medium) && getMediaFile().has_value()) {
        return TrackType::audio;
    }
    return TrackType::noMedia;
}

auto Track::getMediaFile() const -> std::optional<std::filesystem::path>
{
    return std::visit(utl::overloaded{[this](const CardPackPtr& cardPack) -> std::optional<std::filesystem::path> {
                                          return cardPack->getCardByIndex(index).audioFile;
                                      },
                                      [](const VideoPtr& video) -> std::optional<std::filesystem::path> {
                                          return video->getVideoFile();
                                      }},
                      medium);
}

auto Track::getStartTimeStamp() const -> double
{
    return startTimeStamp;
}

auto Track::getEndTimeStamp() const -> double
{
    return endTimeStamp;
}

void Track::setupTimeStamps()
{
    std::visit(utl::overloaded{[this](const CardPackPtr& cardPack) {
                                   const auto& cardAudio = cardPack->getCardByIndex(index);
                                   startTimeStamp = cardAudio.start;
                                   endTimeStamp = cardAudio.end;
                               },
                               [this](const VideoPtr& video) {
                                   const auto& joinedSub = video->getActiveSubtitle()->getJoinedSubAt(index);
                                   startTimeStamp = joinedSub.startTimeStamp;
                                   endTimeStamp = joinedSub.endTimeStamp;
                               }},
               medium);
}

} // namespace database
