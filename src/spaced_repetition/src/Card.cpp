#include <Card.h>
#include <nlohmann/json.hpp>
#include "Time.h"

using namespace spaced_repetition;

void Card::ViewNow() {
    lastSeen = std::time(nullptr);
    viewCount++;
}

auto Card::toJson(const pair_t& pair) -> nlohmann::json {
    const Card& cardSR = pair.second;
    return {{std::string(s_id), pair.first},
            {std::string(s_last_seen), serialize_time_t(cardSR.lastSeen)},
            {std::string(s_view_count), cardSR.viewCount}};
}

auto Card::fromJson(const nlohmann::json& c) -> pair_t {
    return {c.at(std::string(s_id)),
            {.lastSeen = deserialize_time_t(c.at(std::string(s_last_seen))),
             .viewCount = c.at(std::string(s_view_count))}};
}