#include "SpacedRepetitionData.h"

#include <misc/Identifier.h>
#include <utils/Time.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <magic_enum.hpp>
#include <ratio>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ranges = std::ranges;

namespace database {
// auto SpacedRepetitionData::fromVocableProgress(const VocableProgress& progress) -> SpacedRepetitionData
// {
//     // double stability = 0;
//     using namespace std::numbers;
//     if (!progress.isEnabled()) {
//         return {};
//     }
//     constexpr auto square = 2.F;
//     float minFactor = std::pow(Ease::changeFactorHard, square);
//     float maxFactor = progress.easeFactor * Ease::changeFactorHard;
//     float daysMinAtleast = std::clamp(progress.intervalDay, 0.F, 1.F);
//     // auto repRange = RepeatRange{
//     //         .daysMin = utl::daysFromToday(progress.lastSeen,
//     //                                       std::max(daysMinAtleast, progress.intervalDay * minFactor)),
//     //         .daysNormal = progress.dueDays(),
//     //         .daysMax = utl::daysFromToday(progress.lastSeen, progress.intervalDay * maxFactor)};
//
//     auto newShiftBackward = std::max<double>(0, progress.intervalDay - (progress.intervalDay * minFactor));
//     auto newShiftForward = std::max<double>(0, (progress.intervalDay * maxFactor) - progress.intervalDay);
//     return {
//             .reviewed = std::chrono::system_clock::from_time_t(progress.lastSeen),
//             .due = std::chrono::time_point_cast<std::chrono::days>(std::chrono::system_clock::from_time_t(utl::advanceTimeByDays(progress.lastSeen, progress.intervalDay))),
//             .shiftBackward = static_cast<int>(newShiftBackward),
//             .shiftForward = static_cast<int>(newShiftForward),
//             .ease = 1.,
//             .stability = progress.intervalDay == 0 ? 0 : std::log(static_cast<double>(progress.intervalDay)) / std::log(2),
//             .state = StudyState::review,
//             .enabled = progress.enabled,
//             .triggerCardIndices = progress.triggerCardIndices,
//
//     };
// }

auto SpacedRepetitionData::serialize() const -> std::string
{
    // time_point reviewed;
    // time_point due;
    // int shiftBackward{};
    // int shiftForward{};
    // double ease{};
    // double stability{};
    // StudyState state{StudyState::newWord};
    // bool enabled{false};
    // std::vector<std::size_t> triggerCardIndices;
    return fmt::format("{},{},{},{},{:.4F},{:.4F},{},{},{},",
                       utl::serializeTimePoint(reviewed),
                       utl::serializeTimePoint(due),
                       shiftBackward,
                       shiftForward,
                       ease,
                       stability,
                       state,
                       enabled,
                       fmt::join(triggerCardIndices, ","));
}

auto SpacedRepetitionData::deserialize(std::string_view sv) -> SpacedRepetitionData
{
    return {
            .reviewed = utl::deserializeTimePoint(std::string{utl::split_front(sv, ',')}),
            .due = utl::deserializeTimePoint(std::string{utl::split_front(sv, ',')}),
            .shiftBackward = std::stoi(std::string{utl::split_front(sv, ',')}),
            .shiftForward = std::stoi(std::string{utl::split_front(sv, ',')}),
            .ease = std::stof(std::string{utl::split_front(sv, ',')}),
            .stability = std::stof(std::string{utl::split_front(sv, ',')}),
            .state = [&sv]() -> StudyState {
                const auto& studyStateString = std::string{utl::split_front(sv, ',')};
                auto optStudyState = magic_enum::enum_cast<StudyState>(studyStateString);
                if (!optStudyState.has_value()) {
                    throw std::runtime_error(fmt::format("desialize spaced repitiion data, unknown study state: {}",
                                                         studyStateString));
                }
                return optStudyState.value();
            }(),
            .enabled = std::string{utl::split_front(sv, ',')} == "true",
            .triggerCardIndices = [&sv]() -> std::vector<std::size_t> {
                std::vector<std::size_t> triggerCardIndices;
                while (true) {
                    auto cardIndex = std::string{utl::split_front(sv, ',')};
                    if (cardIndex.empty()) {
                        break;
                    }
                    triggerCardIndices.push_back(std::stoul(cardIndex));
                }
                return triggerCardIndices;
            }(),
    };
}

auto SpacedRepetitionData::getDueInTimeLabel() const -> std::string
{
    auto duration = due - reviewed;
    using Months = std::chrono::duration<double, std::ratio<2629746>>;
    using Years = std::chrono::duration<double, std::ratio<31556952>>;
    auto postfix = fmt::format(" - -{} +{} - d: {:.2F}, s: {:.2F}", shiftBackward, shiftForward, ease, stability);
    // const std::string postfix;
    if (const auto& minutes = std::chrono::duration_cast<std::chrono::minutes>(duration); minutes.count() < 60) {
        return fmt::format("{} min", minutes.count()) + postfix;
    }
    if (const auto& hours = std::chrono::duration_cast<std::chrono::hours>(duration); hours.count() < 24) {
        return fmt::format("{} hours", hours.count()) + postfix;
    }
    if (const auto& days = std::chrono::duration_cast<std::chrono::days>(duration); days.count() < 30) {
        return fmt::format("{} days", days.count()) + postfix;
    }
    if (const auto& months = std::chrono::duration_cast<Months>(duration); months.count() <= 12) {
        return fmt::format("{:.1F} months", months.count()) + postfix;
    }
    const auto& years = std::chrono::duration_cast<Years>(duration);
    return fmt::format("{:.1F} years", years.count()) + postfix;
}

void SpacedRepetitionData::triggeredBy(CardId cardId, const std::vector<CardId>& availableCardIds)
{
    auto cardIdit = ranges::find(availableCardIds, cardId);
    auto cardIdindex = static_cast<std::size_t>(std::distance(availableCardIds.begin(), cardIdit));
    auto it = ranges::find(triggerCardIndices, cardIdindex);
    if (it != triggerCardIndices.end()) {
        triggerCardIndices.erase(it);
    }
    triggerCardIndices.push_back(cardIdindex);
}

auto SpacedRepetitionData::triggerValue(CardId cardId, const std::vector<CardId>& availableCardIds) -> std::size_t
{
    auto cardIdit = ranges::find(availableCardIds, cardId);
    auto cardIdindex = static_cast<std::size_t>(std::distance(availableCardIds.begin(), cardIdit));
    auto it = ranges::find(triggerCardIndices, cardIdindex);
    if (it != triggerCardIndices.end()) {
        return triggerCardIndices.size();
    }
    return static_cast<std::size_t>(std::distance(triggerCardIndices.begin(), it));
}

auto SpacedRepetitionData::getNextTriggerCard(const std::vector<CardId>& availableCardIds) const -> CardId
{
    std::vector<std::size_t> triggerCardsTemp;
    ranges::copy_if(triggerCardIndices, std::back_inserter(triggerCardsTemp),
                    [maxIndex = availableCardIds.size() - 1](std::size_t index) -> bool {
                        return index <= maxIndex;
                    });

    std::vector<CardId> triggerCardIds;
    ranges::transform(triggerCardsTemp, std::back_inserter(triggerCardIds),
                      [&availableCardIds](std::size_t index) { return availableCardIds.at(index); });
    CardId result{};
    if (triggerCardsTemp.size() == availableCardIds.size()) {
        result = availableCardIds.at(triggerCardsTemp.front());
        spdlog::info("Triggered so far by [{}], now triggering: {} (old card)", fmt::join(triggerCardIds, ", "), result);
        return result;
    }
    ranges::sort(triggerCardsTemp);
    std::size_t cardIndex = 0;
    for (auto index : triggerCardsTemp) {
        if (cardIndex != index) {
            break;
        }
        cardIndex++;
    }
    result = availableCardIds.at(cardIndex);
    spdlog::info("Triggered so far by [{}], now triggering: {} (new avaliable)", fmt::join(triggerCardIds, ", "), result);
    return result;
}

auto SpacedRepetitionData::pauseTimeOver() const -> bool
{
    return std::chrono::system_clock::now() > due;
}

auto SpacedRepetitionData::recency() const -> double
{
    using Days = std::chrono::duration<double, std::ratio<86400>>;
    auto interval = due - reviewed;
    if (std::chrono::duration_cast<std::chrono::days>(interval).count() == 0) {
        // spdlog::info("click, {}, ------- interval: {}", serialize(),
        //              std::chrono::duration_cast<std::chrono::days>(interval).count());
        return std::chrono::duration_cast<Days>(std::chrono::system_clock::now() - due).count();
    }
    // spdlog::info("unclick, {}, ------- interval: {}", serialize(),
    //              std::chrono::duration_cast<std::chrono::days>(interval).count());
    return stability + 1024;
}

auto SpacedRepetitionData::getRepeatRange() const -> RepeatRange
{
    if (state != StudyState::review) {
        return {.daysMin = 0,
                .daysNormal = 0,
                .daysMax = 0};
    }
    auto duration = utl::setHourOfDay(due,1) - utl::setHourOfDay(std::chrono::system_clock::now(), 0);
    int interval = static_cast<int>(std::chrono::duration_cast<std::chrono::days>(duration).count());
    // int daysMinAtleast = std::clamp(interval, 0, 1);

    return {.daysMin = interval - shiftBackward,
            .daysNormal = interval,
            .daysMax = interval + shiftForward};
    // return {.daysMin = utl::daysFromToday(lastSeen,
    //                                       std::max(daysMinAtleast, intervalDay * minFactor)),
    //         .daysNormal = dueDays(),
    //         .daysMax = utl::daysFromToday(lastSeen, intervalDay * maxFactor)};
}

auto SpacedRepetitionData::RepeatRange::implies(const RepeatRange& other) const -> bool
{
    return std::max(0, daysMax) >= other.daysMin;
}

void SpacedRepetitionData::RepeatRange::debug() const
{
    spdlog::info("RepeatRange: min: {}, normal: {}, max: {}", daysMin, daysNormal, daysMax);
}

auto SpacedRepetitionData::RepeatRange::operator<=>(const RepeatRange& other) const -> std::weak_ordering
{
    if (daysMax < other.daysMax) {
        return std::weak_ordering::less;
    }
    if (daysMax > other.daysMax) {
        return std::weak_ordering::greater;
    }
    if (daysNormal < other.daysNormal) {
        return std::weak_ordering::less;
    }
    if (daysNormal > other.daysNormal) {
        return std::weak_ordering::greater;
    }
    if (daysMin < other.daysMin) {
        return std::weak_ordering::less;
    }
    if (daysMin > other.daysMin) {
        return std::weak_ordering::greater;
    }
    return std::weak_ordering::equivalent;
}
} // namespace database
