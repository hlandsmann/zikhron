#include <Vocable.h>
#include <nlohmann/json.hpp>
#include "Time.h"
using namespace spaced_repetition;

void Vocable::advanceByEase(Ease ease) {
    lastSeen = std::time(nullptr);
    indirectView = std::time(nullptr);

    auto progress = ease.getProgress();
    intervalDay = progress.intervalDay;
    easeFactor = progress.easeFactor;
    indirectIntervalDay = progress.indirectIntervalDay;
}

auto Vocable::advanceIndirectly() -> bool {
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

auto Vocable::urgency() const -> float {
    return (easeFactor * intervalDay) + static_cast<float>(indirectIntervalDay);
}

auto Vocable::pauseTimeOver() const -> bool {
    std::tm last = *std::localtime(&lastSeen);
    last.tm_min += pause_time_minutes;
    std::time_t last_time = std::mktime(&last);
    std::time_t now_time = std::time(nullptr);

    return last_time < now_time;
}

auto Vocable::fromJson(const nlohmann::json& jsonIn) -> pair_t {
    Init init = {.easeFactor = jsonIn.at(std::string(Vocable::s_ease_factor)),
                 .intervalDay = jsonIn.at(std::string(Vocable::s_interval_day)),
                 .lastSeen = deserialize_time_t(jsonIn.at(std::string(Vocable::s_last_seen))),
                 .indirectView = deserialize_time_t(jsonIn.at(std::string(Vocable::s_indirect_view))),
                 .indirectIntervalDay = jsonIn.at(std::string(Vocable::s_indirect_interval_day))};
    return {jsonIn.at(std::string(Vocable::s_id)), {init}};
}

auto Vocable::toJson(const pair_t& pair) -> nlohmann::json {
    const Vocable& vocSR = pair.second;
    return {{std::string(Vocable::s_id), pair.first},
            {std::string(Vocable::s_ease_factor), vocSR.easeFactor},
            {std::string(Vocable::s_interval_day), vocSR.intervalDay},
            {std::string(Vocable::s_last_seen), serialize_time_t(vocSR.lastSeen)},
            {std::string(Vocable::s_indirect_view), serialize_time_t(vocSR.indirectView)},
            {std::string(Vocable::s_indirect_interval_day), vocSR.indirectIntervalDay}};
}

auto Vocable::isToBeRepeatedToday() const -> bool {
    std::time_t todayMidnight = todayMidnightTime();
    std::time_t vocActiveTime = advanceTimeByDays(lastSeen,
                                                  intervalDay + static_cast<float>(indirectIntervalDay));

    return todayMidnight > vocActiveTime;
}

auto Vocable::isAgainVocable() const -> bool { return intervalDay == 0; };

auto Vocable::getRepeatRange() const -> RepeatRange {
    constexpr auto square = 2.F;
    float minFactor = std::pow(Ease::changeFactorHard, square);
    float maxFactor = easeFactor * Ease::changeFactorHard;
    return {.daysMin = daysFromNow(lastSeen, intervalDay * minFactor),
            .daysNormal = daysFromNow(lastSeen, intervalDay),
            .daysMax = daysFromNow(lastSeen, intervalDay * maxFactor)};
}
