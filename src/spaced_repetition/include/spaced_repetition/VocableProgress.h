#pragma once

#include <annotation/Ease.h>

#include <compare>
#include <ctime>
#include <nlohmann/json_fwd.hpp> // IWYU pragma: export
#include <string_view>
#include <utility>

class VocableProgress
{
public:
    struct Init
    {
        float easeFactor = 0.F;
        float intervalDay = 0.F;
        std::time_t lastSeen{};
        std::time_t indirectView{};
        int indirectIntervalDay = 0;
    };
    VocableProgress(Init init)
        : easeFactor{init.easeFactor}
        , intervalDay{init.intervalDay}
        , lastSeen{init.lastSeen}
        , indirectView{init.indirectView}
        , indirectIntervalDay{init.indirectIntervalDay} {}
    VocableProgress()
        : VocableProgress(Init{}) {}
    struct RepeatRange
    {
        int daysMin;
        int daysNormal;
        int daysMax;
        [[nodiscard]] auto implies(const RepeatRange&) const -> bool;
        auto operator<=>(const RepeatRange&) const -> std::weak_ordering;
    };
    static constexpr int pause_time_minutes = 5;
    using pair_t = std::pair<unsigned, VocableProgress>;
    static constexpr std::string_view s_id = "id";
    static constexpr std::string_view s_ease_factor = "ease_factor";
    static constexpr std::string_view s_interval_day = "interval_day";
    static constexpr std::string_view s_last_seen = "last_seen";
    static constexpr std::string_view s_indirect_view = "indirect_view";
    static constexpr std::string_view s_indirect_interval_day = "indirect_interval_day";

    void advanceByEase(Ease);
    auto advanceIndirectly() -> bool;
    [[nodiscard]] auto recency() const -> float;
    [[nodiscard]] auto urgency() const -> float;
    [[nodiscard]] auto pauseTimeOver() const -> bool;
    [[nodiscard]] auto isToBeRepeatedToday() const -> bool;
    [[nodiscard]] auto isAgainVocable() const -> bool;
    [[nodiscard]] auto getRepeatRange() const -> RepeatRange;
    [[nodiscard]] auto IntervalDay() const -> float { return intervalDay; }
    [[nodiscard]] auto EaseFactor() const -> float { return easeFactor; }
    [[nodiscard]] auto IndirectIntervalDay() const -> int { return indirectIntervalDay; }

    static auto toJson(const pair_t&) -> nlohmann::json;
    static auto fromJson(const nlohmann::json&) -> pair_t;

private:
    float easeFactor = 0.F;
    float intervalDay = 0.F;
    std::time_t lastSeen{};
    std::time_t indirectView{};
    int indirectIntervalDay = 0;
};