#include "Time.h"
#include <chrono>
namespace chrono = std::chrono;
namespace spaced_repetition {

auto serialize_time_t(const std::time_t& time) -> std::string {
    std::stringstream transTime;
    transTime << std::put_time(std::localtime(&time), "%F %T");
    return transTime.str();
}

auto deserialize_time_t(const std::string& s) -> std::time_t {
    std::tm time{};
    time.tm_isdst = -1;
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

auto advanceTimeByDays(std::time_t inputTime, float days) -> std::time_t {
    std::tm vocActiveTime_tm = *std::localtime(&inputTime);
    vocActiveTime_tm.tm_mday += static_cast<int>(days);
    return std::mktime(&vocActiveTime_tm);
}

auto daysFromNow(std::time_t startTime, float intervalDays) -> int {
    const auto sysclock_now = chrono::system_clock::from_time_t(std::time(nullptr));
    const auto sysclock_startTime = chrono::system_clock::from_time_t(
        advanceTimeByDays(startTime, intervalDays));
    const auto timePoint_now = chrono::time_point_cast<chrono::days>(sysclock_now);
    const auto timePoint_due = chrono::time_point_cast<chrono::days>(sysclock_startTime);
    return static_cast<int>((chrono::sys_days{timePoint_due} - chrono::sys_days{timePoint_now}).count());
}

}  // namespace spaced_repitition
