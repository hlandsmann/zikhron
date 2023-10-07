#pragma once
#include <utility>
#include <misc/Identifier.h>
#include <string_view>
#include <ctime>
#include <nlohmann/json_fwd.hpp>

struct CardProgress {
    using pair_t = std::pair<CardId, CardProgress>;
    static constexpr std::string_view s_id = "id";
    static constexpr std::string_view s_last_seen = "last_seen";
    static constexpr std::string_view s_view_count = "view_count";

    std::time_t lastSeen{};
    unsigned viewCount{};

    void ViewNow();
    static auto toJson(const pair_t&) -> nlohmann::json;
    static auto fromJson(const nlohmann::json&) -> pair_t;
};
