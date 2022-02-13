#pragma once

#include <nlohmann/json_fwd.hpp>
#include <set>
#include <ctime>
#include <annotation/Ease.h>

struct VocableMeta {
    // uint id = 0;
    std::set<uint> cardIds;
};

struct CardMeta {
    float value = 0;
    std::set<uint> vocableIds;

    uint cardId = 0;
};

struct VocableSR {
    static constexpr int pause_time_minutes = 5;
    using pair_t = std::pair<uint, VocableSR>;
    static constexpr std::string_view s_id = "id";
    static constexpr std::string_view s_ease_factor = "ease_factor";
    static constexpr std::string_view s_interval_day = "interval_day";
    static constexpr std::string_view s_last_seen = "last_seen";
    static constexpr std::string_view s_indirect_view = "indirect_view";
    static constexpr std::string_view s_indirect_interval_day = "indirect_interval_day";

    float easeFactor = 0.f;
    float intervalDay = 0.f;
    std::time_t lastSeen{};
    std::time_t indirectView{};
    uint indirectIntervalDay = 0;

    void advanceByEase(Ease);
    bool advanceIndirectly();
    auto urgency() const -> float;
    auto pauseTimeOver() const -> bool;
    auto isToBeRepeatedToday() const -> bool;
    auto isAgainVocable() const -> bool;
    static auto toJson(const pair_t&) -> nlohmann::json;
    static auto fromJson(const nlohmann::json&) -> pair_t;
};

struct CardSR {
    using pair_t = std::pair<uint, CardSR>;
    static constexpr std::string_view s_id = "id";
    static constexpr std::string_view s_last_seen = "last_seen";
    static constexpr std::string_view s_view_count = "view_count";

    std::time_t lastSeen{};
    uint viewCount{};

    void ViewNow();
    static auto toJson(const pair_t&) -> nlohmann::json;
    static auto fromJson(const nlohmann::json&) -> pair_t;
};
