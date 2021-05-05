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

auto CardSR::toJson() const -> nlohmann::json {
    nlohmann::json c;
    c[std::string(s_id)] = cardId;
    c[std::string(s_last_seen)] = serialize_time_t(last_seen);
    return c;
}

auto CardSR::fromJson(const nlohmann::json& c) -> CardSR {
    return {.cardId = c.at(std::string(s_id)),
            .last_seen = deserialize_time_t(c.at(std::string(s_last_seen)))};
}

void VocableSR::advanceByEase(Ease ease) {
    if (ease == Ease::again) {
        interval_day = 0;
    } else {
        interval_day = std::max(1.f, interval_day * ease_factor);
    }
}

auto VocableSR::fromJson(const nlohmann::json& jsonIn) -> VocableSR {
    return {.vocId = jsonIn.at(std::string(VocableSR::s_id)),
            .ease_factor = jsonIn.at(std::string(VocableSR::s_ease_factor)),
            .interval_day = jsonIn.at(std::string(VocableSR::s_interval_day)),
            .last_seen = deserialize_time_t(jsonIn.at(std::string(VocableSR::s_last_seen)))};
}

auto VocableSR::toJson() const -> nlohmann::json {
    nlohmann::json v;
    v[std::string(VocableSR::s_id)] = vocId;
    v[std::string(VocableSR::s_ease_factor)] = ease_factor;
    v[std::string(VocableSR::s_interval_day)] = interval_day;
    v[std::string(VocableSR::s_last_seen)] = serialize_time_t(last_seen);
    return v;
}
