#include "Track.h"

#include "Card.h" // IWYU pragma: keep
#include "CardPack.h"
#include "CbdFwd.h"
#include "SubtitlePicker.h"
#include "Video.h"

#include <misc/Identifier.h>
#include <multimedia/Subtitle.h>
#include <spdlog/spdlog.h>
#include <utils/Variant.h>

#include <cstddef>
#include <exception>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>

namespace database {

Track::Track(TrackMedia _medium, CardPtr _card)
    : medium{std::move(_medium)}
    , card{std::move(_card)}
{
    try {
        setupMedium();
    } catch (const std::exception& e) {
        spdlog::error("track, e.what: {}", e.what());
    }
}

Track::Track(const TrackMedia& _medium, std::size_t index)
    : Track{_medium, getCard(_medium, index)}
{
}

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

auto Track::continueTrack() const -> Track
{
    return std::visit(utl::overloaded{[this](const CardPackPtr& /* cardPack */) -> Track {
                                          auto index = card->getIndexInPack();
                                          return {medium, index};
                                      },
                                      [this](const VideoPtr& video) -> Track {
                                          const auto& subtitlePicker = video->getActiveSubtitle();
                                          auto nextJoinedSubtitle = subtitlePicker->joinedSubtitleFromLastActiveCard();
                                          return {medium, nextJoinedSubtitle};
                                      }},
                      medium);
}

auto Track::nextTrack() const -> Track
{
    return std::visit(utl::overloaded{[this](const CardPackPtr& /* cardPack */) -> Track {
                                          auto index = card->getIndexInPack();
                                          return {medium, index + 1};
                                      },
                                      [this](const VideoPtr& video) -> Track {
                                          const auto& subtitlePicker = video->getActiveSubtitle();
                                          auto nextJoinedSubtitle = subtitlePicker->getNext(card);
                                          return {medium, nextJoinedSubtitle};
                                      }},
                      medium);
}

auto Track::previousTrack() const -> Track
{
    return std::visit(utl::overloaded{[this](const CardPackPtr& /* cardPack */) -> Track {
                                          auto index = card->getIndexInPack();
                                          return {medium, index - 1};
                                      },
                                      [this](const VideoPtr& video) -> Track {
                                          const auto& subtitlePicker = video->getActiveSubtitle();
                                          auto previousJoinedSubtitle = subtitlePicker->getPrevious(card);
                                          return {medium, previousJoinedSubtitle};
                                      }},
                      medium);
}

auto Track::hasNext() const -> bool
{
    return std::visit(utl::overloaded{[this](const CardPackPtr& /* cardPack */) -> bool {
                                          auto index = card->getIndexInPack();
                                          return index + 1 < numberOfTracks();
                                      },
                                      [this](const VideoPtr& video) -> bool {
                                          const auto& subtitlePicker = video->getActiveSubtitle();
                                          return subtitlePicker->hasNext(card);
                                      }},
                      medium);
}

auto Track::hasPrevious() const -> bool
{
    return std::visit(utl::overloaded{[this](const CardPackPtr& /* cardPack */) -> bool {
                                          auto index = card->getIndexInPack();
                                          return index > 0;
                                      },
                                      [this](const VideoPtr& video) -> bool {
                                          const auto& subtitlePicker = video->getActiveSubtitle();
                                          return subtitlePicker->hasPrevious(card);
                                      }},
                      medium);
}

auto Track::isFrontJoinable() const -> bool
{
    return std::visit(utl::overloaded{[](const CardPackPtr& /* cardPack */) -> bool {
                                          return false;
                                      },
                                      [this](const VideoPtr& video) -> bool {
                                          return video->getActiveSubtitle()->isFrontJoinable(card);
                                      }},
                      medium);
}

auto Track::isBackJoinable() const -> bool
{
    return std::visit(utl::overloaded{[](const CardPackPtr& /* cardPack */) -> bool {
                                          return false;
                                      },
                                      [this](const VideoPtr& video) -> bool {
                                          return video->getActiveSubtitle()->isBackJoinable(card);
                                      }},
                      medium);
}

auto Track::isSeparable() const -> bool
{
    return std::visit(utl::overloaded{[](const CardPackPtr& /* cardPack */) -> bool {
                                          return false;
                                      },
                                      [this](const VideoPtr& video) -> bool {
                                          return video->getActiveSubtitle()->isSeparable(card);
                                      }},
                      medium);
}

auto Track::joinFront() const -> Track
{
    const auto& video = std::get<VideoPtr>(medium);
    auto subtitlePicker = video->getActiveSubtitle();
    auto joinedSubtitle = subtitlePicker->joinFront(card);
    return {medium, joinedSubtitle};
}

auto Track::joinBack() const -> Track
{
    const auto& video = std::get<VideoPtr>(medium);
    auto subtitlePicker = video->getActiveSubtitle();
    auto joinedSubtitle = subtitlePicker->joinBack(card);
    return {medium, joinedSubtitle};
}

auto Track::cutFront() const -> Track
{
    const auto& video = std::get<VideoPtr>(medium);
    auto subtitlePicker = video->getActiveSubtitle();
    auto joinedSubtitle = subtitlePicker->cutFront(card);
    return {medium, joinedSubtitle};
}

auto Track::cutBack() const -> Track
{
    const auto& video = std::get<VideoPtr>(medium);
    auto subtitlePicker = video->getActiveSubtitle();
    auto joinedSubtitle = subtitlePicker->cutBack(card);
    return {medium, joinedSubtitle};
}

auto Track::getCard() const -> CardPtr
{
    return card;
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
                                          auto index = card->getIndexInPack();
                                          return cardPack->getCardAudioByIndex(index).audioFile;
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

auto Track::hasSubtitlePrefix() const -> bool
{
    if (isSubtitlePrefix()) {
        return false;
    }

    auto [startTime, endTime] = getSubtitlePrefixTime();
    return startTime < endTime;
}

auto Track::getSubtitlePrefix() const -> Track
{
    auto [newStartTime, newEndTime] = getSubtitlePrefixTime();
    return {medium, card, newStartTime, newEndTime};
}

auto Track::getSubtitlePrefixTime() const -> std::pair<double, double>
{
    const auto& video = std::get<VideoPtr>(medium);
    auto subtitlePicker = video->getActiveSubtitle();
    double newStartTime{0.0F};
    double newEndTime{startTimeStamp - subtitlePicker->getTimeExtraFront(card)};
    if (subtitlePicker->hasPrevious(card)) {
        auto previousJoinedSubtitle = subtitlePicker->getPrevious(card);
        auto previousCard = previousJoinedSubtitle.card;
        newStartTime = previousJoinedSubtitle.endTimeStamp + subtitlePicker->getTimeExtraBack(previousCard);
    }
    return {newStartTime, newEndTime};
}

auto Track::getNonPrefixDefault() const -> Track
{
    return {medium, card};
}

auto Track::isSubtitlePrefix() const -> bool
{
    return isPrefixToSubtitle;
}

auto Track::getTranslation() const -> std::optional<std::string>
{
    return std::visit(utl::overloaded{[](const CardPackPtr& /* cardPack */) -> std::optional<std::string> {
                                          return {};
                                      },
                                      [this](const VideoPtr& video) -> std::optional<std::string> {
                                          const auto& translation = video->getTranslation();
                                          if (!translation.has_value()) {
                                              return {};
                                          }
                                          return (*translation)->get(startTimeStamp, endTimeStamp);
                                      }},
                      medium);
}

auto Track::timeAddBack() const -> Track
{
    const auto& video = std::get<VideoPtr>(medium);
    auto subtitlePicker = video->getActiveSubtitle();
    subtitlePicker->timeAddBack(card);
    auto joinedSubtitle = subtitlePicker->joinFront(card);
    return {medium, joinedSubtitle};
}

auto Track::timeAddFront() const -> Track
{
    const auto& video = std::get<VideoPtr>(medium);
    auto subtitlePicker = video->getActiveSubtitle();
    subtitlePicker->timeAddFront(card);
    auto joinedSubtitle = subtitlePicker->joinFront(card);
    return {medium, joinedSubtitle};
}

auto Track::timeDelBack() const -> Track
{
    const auto& video = std::get<VideoPtr>(medium);
    auto subtitlePicker = video->getActiveSubtitle();
    subtitlePicker->timeDelBack(card);
    auto joinedSubtitle = subtitlePicker->joinFront(card);
    return {medium, joinedSubtitle};
}

auto Track::timeDelFront() const -> Track
{
    const auto& video = std::get<VideoPtr>(medium);
    auto subtitlePicker = video->getActiveSubtitle();
    subtitlePicker->timeDelFront(card);
    auto joinedSubtitle = subtitlePicker->joinFront(card);
    return {medium, joinedSubtitle};
}

auto Track::getTimeExtraBack() const -> double
{
    const auto& video = std::get<VideoPtr>(medium);
    auto subtitlePicker = video->getActiveSubtitle();
    return subtitlePicker->getTimeExtraBack(card);
}

auto Track::getTimeExtraFront() const -> double
{
    const auto& video = std::get<VideoPtr>(medium);
    auto subtitlePicker = video->getActiveSubtitle();
    return subtitlePicker->getTimeExtraFront(card);
}

Track::Track(TrackMedia _medium, const JoinedSubtitle& joinedSubtitle)
    : medium{std::move(_medium)}
{
    setupJoinedSubtitle(joinedSubtitle);
}

// Prefix ctor
Track::Track(TrackMedia _medium, CardPtr _card, double _startTimeStamp, double _endTimeStamp)
    : medium{std::move(_medium)}
    , card{std::move(_card)}
    , startTimeStamp{_startTimeStamp}
    , endTimeStamp{_endTimeStamp}
    , isPrefixToSubtitle{true}
{
    const auto& video = std::get<VideoPtr>(medium);
    auto subtitlePicker = video->getActiveSubtitle();

    spdlog::info("sts: {}", startTimeStamp);
}

auto Track::getCard(TrackMedia medium, std::size_t index) -> CardPtr
{
    return std::visit(utl::overloaded{[index](const CardPackPtr& cardPack) -> CardPtr {
                                          return cardPack->getCardAudioByIndex(index).card;
                                      },
                                      [index](const VideoPtr& video) -> CardPtr {
                                          return video->getActiveSubtitle()->getJoinedSubAtPosition(index).card;
                                      }},
                      medium);
}

void Track::setupMedium()
{
    std::visit(utl::overloaded{[this](const CardPackPtr& cardPack) {
                                   auto index = card->getIndexInPack();
                                   const auto& cardAudio = cardPack->getCardAudioByIndex(index);
                                   setupCardAudio(cardAudio);
                               },
                               [this](const VideoPtr& video) {
                                   const auto& joinedSubtitle = video->getActiveSubtitle()->joinedSubtitleFromCard(card);
                                   setupJoinedSubtitle(joinedSubtitle);
                               }},
               medium);
}

void Track::setupCardAudio(const CardAudio& cardAudio)
{
    startTimeStamp = cardAudio.start;
    endTimeStamp = cardAudio.end;
    card = cardAudio.card;
}

void Track::setupJoinedSubtitle(const JoinedSubtitle& joinedSubtitle)
{
    startTimeStamp = joinedSubtitle.startTimeStamp;
    endTimeStamp = joinedSubtitle.endTimeStamp;
    card = joinedSubtitle.card;

    const auto& video = std::get<VideoPtr>(medium);
    auto subtitlePicker = video->getActiveSubtitle();
    startTimeStamp -= subtitlePicker->getTimeExtraFront(card);
    endTimeStamp += subtitlePicker->getTimeExtraBack(card);
}

} // namespace database
