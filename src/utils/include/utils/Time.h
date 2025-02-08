#pragma once
#include <chrono>
#include <ctime>
#include <string>

namespace utl {

auto serializeTimePoint(const std::chrono::time_point<std::chrono::system_clock>& timePoint) -> std::string;
auto deserializeTimePoint(const std::string& s) -> std::chrono::time_point<std::chrono::system_clock>;
auto serialize_time_t(const std::time_t& time) -> std::string;
auto deserialize_time_t(const std::string& s) -> std::time_t;
auto todayMidnightTime() -> std::time_t;
auto todayMorningTime() -> std::time_t;
auto advanceTimeByDays(std::time_t inputTime, float days) -> std::time_t;
auto daysFromToday(std::time_t startTime, float intervalDays) -> int;

} // namespace utl
