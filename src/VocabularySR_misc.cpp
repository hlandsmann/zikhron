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

    if (ease == Ease::again) {
        intervalDay = 0;
    } else {
        intervalDay = std::max(1.f, intervalDay * easeFactor);
        if (ease == Ease::good)
            intervalDay += 1 * easeFactor;
        if (ease == Ease::easy)
            intervalDay += 2 * easeFactor;
    }
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
             .lastSeen = deserialize_time_t(jsonIn.at(std::string(VocableSR::s_last_seen)))}};
}

auto VocableSR::toJson(const pair_t& pair) -> nlohmann::json {
    const VocableSR& vocSR = pair.second;
    return {{std::string(VocableSR::s_id), pair.first},
            {std::string(VocableSR::s_ease_factor), vocSR.easeFactor},
            {std::string(VocableSR::s_interval_day), vocSR.intervalDay},
            {std::string(VocableSR::s_last_seen), serialize_time_t(vocSR.lastSeen)}};
}
