#pragma once
#include <ctime>
#include <string>
namespace spaced_repetition {

auto serialize_time_t(const std::time_t& time) -> std::string;
auto deserialize_time_t(const std::string& s) -> std::time_t;
auto todayMidnightTime() -> std::time_t;
auto advanceTimeByDays(std::time_t inputTime, float days) -> std::time_t;
auto daysFromNow(std::time_t startTime, float intervalDays) -> int;

}  // namespace spaced_repitition