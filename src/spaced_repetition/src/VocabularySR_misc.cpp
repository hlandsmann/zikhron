#include <fmt/format.h>
#include <fmt/ostream.h>
#include <ctime>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>
#include "VocabularySR.h"

namespace {

auto serialize_time_t(const std::time_t& time) -> std::string {
    return fmt::format("{}", std::put_time(std::localtime(&time), "%F %T"));
}

auto deserialize_time_t(const std::string& s) -> std::time_t {
    std::tm time;
    std::stringstream ss(s);
    ss >> std::get_time(&time, "%Y-%m-%d %H:%M:%S");
    return std::mktime(&time);
}

auto todayMidnightTime() -> std::time_t {
    std::time_t now = std::time(nullptr);
    std::tm todayMidnight_tm = *std::localtime(&now);
    todayMidnight_tm.tm_sec = 0;
    todayMidnight_tm.tm_min = 0;
    todayMidnight_tm.tm_hour = 0;
    todayMidnight_tm.tm_mday += 1;
    return std::mktime(&todayMidnight_tm);
}

}  // namespace

void CardSR::ViewNow() {
    lastSeen = std::time(nullptr);
    viewCount++;
}

auto CardSR::toJson(const pair_t& pair) -> nlohmann::json {
    const CardSR& cardSR = pair.second;
    return {{std::string(s_id), pair.first},
            {std::string(s_last_seen), serialize_time_t(cardSR.lastSeen)},
            {std::string(s_view_count), cardSR.viewCount}};
}

auto CardSR::fromJson(const nlohmann::json& c) -> pair_t {
    return {c.at(std::string(s_id)),
            {.lastSeen = deserialize_time_t(c.at(std::string(s_last_seen))),
             .viewCount = c.at(std::string(s_view_count))}};
}

void VocableSR::advanceByEase(Ease ease) {
    float easeChange = [ease]() -> float {
        switch (ease) {
        case Ease::easy: return 1.2;
        case Ease::good: return 1.0;
        case Ease::hard: return 0.8;
        default: return 0.;
        }
    }();
    easeFactor = std::clamp(easeFactor * easeChange, 1.3f, 2.5f);
    lastSeen = std::time(nullptr);
    indirectView = std::time(nullptr);

    if (ease == Ease::again) {
        intervalDay = 0;
    } else {
        intervalDay = std::max(1.f, intervalDay * easeFactor);
        if (ease == Ease::good)
            intervalDay += 1 * easeFactor;
        if (ease == Ease::easy)
            intervalDay += 2 * easeFactor;

        intervalDay += indirectIntervalDay / 3;
    }
    indirectIntervalDay = 0;
}

auto VocableSR::advanceIndirectly() -> bool {
    if (isToBeRepeatedToday())
        return false;

    bool advanceIntervalDay = false;

    /* Each Vocable is to be advanced only once each day it is viewed */
    std::tm IndirectViewTime_tm = *std::localtime(&indirectView);
    IndirectViewTime_tm.tm_mday += 1;
    std::time_t indirectViewTime = std::mktime(&IndirectViewTime_tm);

    if (indirectViewTime < todayMidnightTime())
        advanceIntervalDay = true;
    indirectView = std::time(nullptr);

    indirectIntervalDay += advanceIntervalDay ? 1 : 0;
    return advanceIntervalDay;
}

auto VocableSR::urgency() const -> float {
    return (easeFactor * intervalDay) + float(indirectIntervalDay);
}

auto VocableSR::pauseTimeOver() const -> bool {
    std::tm last = *std::localtime(&lastSeen);
    last.tm_min += pause_time_minutes;
    std::time_t last_time = std::mktime(&last);
    std::time_t now_time = std::time(nullptr);

    return last_time < now_time;
};

auto VocableSR::fromJson(const nlohmann::json& jsonIn) -> pair_t {
    return {jsonIn.at(std::string(VocableSR::s_id)),
            {.easeFactor = jsonIn.at(std::string(VocableSR::s_ease_factor)),
             .intervalDay = jsonIn.at(std::string(VocableSR::s_interval_day)),
             .lastSeen = deserialize_time_t(jsonIn.at(std::string(VocableSR::s_last_seen))),
             .indirectView = deserialize_time_t(jsonIn.at(std::string(VocableSR::s_indirect_view))),
             .indirectIntervalDay = jsonIn.at(std::string(VocableSR::s_indirect_interval_day))}};
}

auto VocableSR::toJson(const pair_t& pair) -> nlohmann::json {
    const VocableSR& vocSR = pair.second;
    return {{std::string(VocableSR::s_id), pair.first},
            {std::string(VocableSR::s_ease_factor), vocSR.easeFactor},
            {std::string(VocableSR::s_interval_day), vocSR.intervalDay},
            {std::string(VocableSR::s_last_seen), serialize_time_t(vocSR.lastSeen)},
            {std::string(VocableSR::s_indirect_view), serialize_time_t(vocSR.indirectView)},
            {std::string(VocableSR::s_indirect_interval_day), vocSR.indirectIntervalDay}};
}

auto VocableSR::isToBeRepeatedToday() const -> bool {
    std::time_t todayMidnight = todayMidnightTime();

    std::tm vocActiveTime_tm = *std::localtime(&lastSeen);
    vocActiveTime_tm.tm_mday += intervalDay + indirectIntervalDay;
    std::time_t vocActiveTime = std::mktime(&vocActiveTime_tm);

    return todayMidnight > vocActiveTime;
}

auto VocableSR::isAgainVocable() const -> bool { return intervalDay == 0; };
