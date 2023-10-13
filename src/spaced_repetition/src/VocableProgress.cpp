#include <VocableProgress.h>
#include <annotation/Ease.h>

#include <algorithm>
#include <cmath>
#include <compare>
#include <ctime>
#include <nlohmann/json.hpp>
#include <string>

#include "Time.h"
using namespace spaced_repetition;

auto VocableProgress::RepeatRange::implies(const RepeatRange& other) const -> bool
{
    return std::max(0, daysMax) >= other.daysMin;
}

auto VocableProgress::RepeatRange::operator<=>(const RepeatRange& other) const -> std::weak_ordering
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

void VocableProgress::advanceByEase(const Ease& ease)
{
    lastSeen = std::time(nullptr);

    auto progress = ease.getProgress();
    intervalDay = progress.intervalDay;
    easeFactor = progress.easeFactor;
}

auto VocableProgress::recency() const -> float
{
    return (easeFactor * intervalDay) - static_cast<float>(daysFromNow(lastSeen, 0));
}

auto VocableProgress::pauseTimeOver() const -> bool
{
    std::tm last = *std::localtime(&lastSeen);
    last.tm_min += pause_time_minutes;
    std::time_t last_time = std::mktime(&last);
    std::time_t now_time = std::time(nullptr);

    return last_time < now_time;
}

auto VocableProgress::fromJson(const nlohmann::json& jsonIn) -> pair_t
{
    Init init = {.easeFactor = jsonIn.at(std::string(VocableProgress::s_ease_factor)),
                 .intervalDay = jsonIn.at(std::string(VocableProgress::s_interval_day)),
                 .lastSeen = deserialize_time_t(jsonIn.at(std::string(VocableProgress::s_last_seen)))};
    return {jsonIn.at(std::string(VocableProgress::s_id)), {init}};
}

auto VocableProgress::toJson(const pair_t& pair) -> nlohmann::json
{
    const VocableProgress& vocSR = pair.second;
    return {
            {std::string(VocableProgress::s_id), pair.first},
            {std::string(VocableProgress::s_ease_factor), vocSR.easeFactor},
            {std::string(VocableProgress::s_interval_day), vocSR.intervalDay},
            {std::string(VocableProgress::s_last_seen), serialize_time_t(vocSR.lastSeen)}};
}

auto VocableProgress::isToBeRepeatedToday() const -> bool
{
    std::time_t todayMidnight = todayMidnightTime();
    std::time_t vocActiveTime = advanceTimeByDays(lastSeen, intervalDay);

    return todayMidnight > vocActiveTime;
}

auto VocableProgress::isAgainVocable() const -> bool
{
    return intervalDay == 0;
};

auto VocableProgress::getRepeatRange() const -> RepeatRange
{
    constexpr auto square = 2.F;
    float minFactor = std::pow(Ease::changeFactorHard, square);
    float maxFactor = easeFactor * Ease::changeFactorHard;
    return {.daysMin = daysFromNow(lastSeen, intervalDay * minFactor),
            .daysNormal = daysFromNow(lastSeen, intervalDay),
            .daysMax = daysFromNow(lastSeen, intervalDay * maxFactor)};
}
