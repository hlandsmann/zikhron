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
}  // namespace

void CardSR::ViewNow() {
    lastSeen = std::time(nullptr);
    viewCount++;
}

auto CardSR::toJson(const pair_t& pair) -> nlohmann::json {
    nlohmann::json c;
    const CardSR& cardSR = pair.second;
    c[std::string(s_id)] = pair.first;
    c[std::string(s_last_seen)] = serialize_time_t(cardSR.lastSeen);
    c[std::string(s_view_count)] = cardSR.viewCount;
    return c;
}

auto CardSR::fromJson(const nlohmann::json& c) -> pair_t {
    return {c.at(std::string(s_id)),
            {.lastSeen = deserialize_time_t(c.at(std::string(s_last_seen))),
             .viewCount = c.at(std::string(s_view_count))}};
}

void VocableSR::advanceByEase(Ease ease) {
    if (ease == Ease::again) {
        intervalDay = 0;
    } else {
        intervalDay = std::max(1.f, intervalDay * easeFactor);
    }
}

auto VocableSR::fromJson(const nlohmann::json& jsonIn) -> pair_t {
    return {jsonIn.at(std::string(VocableSR::s_id)),
            {.easeFactor = jsonIn.at(std::string(VocableSR::s_ease_factor)),
             .intervalDay = jsonIn.at(std::string(VocableSR::s_interval_day)),
             .lastSeen = deserialize_time_t(jsonIn.at(std::string(VocableSR::s_last_seen)))}};
}

auto VocableSR::toJson(const pair_t& pair) -> nlohmann::json {
    nlohmann::json v;
    const VocableSR& vocSR = pair.second;
    v[std::string(VocableSR::s_id)] = pair.first;
    v[std::string(VocableSR::s_ease_factor)] = vocSR.easeFactor;
    v[std::string(VocableSR::s_interval_day)] = vocSR.intervalDay;
    v[std::string(VocableSR::s_last_seen)] = serialize_time_t(vocSR.lastSeen);
    return v;
}
