#include <VocableProgress.h>

#include <nlohmann/json.hpp>

#include "Time.h"
using namespace spaced_repetition;

auto VocableProgress::RepeatRange::implies(const RepeatRange& other) const -> bool
{
    return std::max(0, daysMax) >= other.daysMin;
}

auto VocableProgress::RepeatRange::operator<=>(const RepeatRange& other) const -> std::weak_ordering
{
    if (daysNormal < other.daysNormal) {
        return std::weak_ordering::less;
    }
    if (daysNormal > other.daysNormal) {
        return std::weak_ordering::greater;
    }
    if (daysMax < other.daysMax) {
        return std::weak_ordering::less;
    }
    if (daysMax > other.daysMax) {
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

void VocableProgress::advanceByEase(Ease ease)
{
    lastSeen = std::time(nullptr);
    indirectView = std::time(nullptr);

    auto progress = ease.getProgress();
    intervalDay = progress.intervalDay;
    easeFactor = progress.easeFactor;
    indirectIntervalDay = progress.indirectIntervalDay;
}

auto VocableProgress::advanceIndirectly() -> bool
{
    if (isToBeRepeatedToday()) {
        return false;
    }

    bool advanceIntervalDay = false;

    /* Each Vocable is to be advanced only once each day it is viewed */
    std::tm IndirectViewTime_tm = *std::localtime(&indirectView);
    IndirectViewTime_tm.tm_mday += 1;
    std::time_t indirectViewTime = std::mktime(&IndirectViewTime_tm);

    if (indirectViewTime < todayMidnightTime()) {
        advanceIntervalDay = true;
    }
    indirectView = std::time(nullptr);

    indirectIntervalDay += advanceIntervalDay ? 1 : 0;
    return advanceIntervalDay;
}

auto VocableProgress::urgency() const -> float
{
    return (easeFactor * intervalDay) + static_cast<float>(indirectIntervalDay);
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
                 .lastSeen = deserialize_time_t(jsonIn.at(std::string(VocableProgress::s_last_seen))),
                 .indirectView = deserialize_time_t(jsonIn.at(std::string(VocableProgress::s_indirect_view))),
                 .indirectIntervalDay = jsonIn.at(std::string(VocableProgress::s_indirect_interval_day))};
    return {jsonIn.at(std::string(VocableProgress::s_id)), {init}};
}

auto VocableProgress::toJson(const pair_t& pair) -> nlohmann::json
{
    const VocableProgress& vocSR = pair.second;
    return {{std::string(VocableProgress::s_id), pair.first},
            {std::string(VocableProgress::s_ease_factor), vocSR.easeFactor},
            {std::string(VocableProgress::s_interval_day), vocSR.intervalDay},
            {std::string(VocableProgress::s_last_seen), serialize_time_t(vocSR.lastSeen)},
            {std::string(VocableProgress::s_indirect_view), serialize_time_t(vocSR.indirectView)},
            {std::string(VocableProgress::s_indirect_interval_day), vocSR.indirectIntervalDay}};
}

auto VocableProgress::isToBeRepeatedToday() const -> bool
{
    std::time_t todayMidnight = todayMidnightTime();
    std::time_t vocActiveTime = advanceTimeByDays(lastSeen,
                                                  intervalDay + static_cast<float>(indirectIntervalDay));

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
