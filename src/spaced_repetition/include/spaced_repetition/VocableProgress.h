#pragma once
#include <string_view>
#include <annotation/Ease.h>
#include <misc/Identifier.h>

#include <compare>
#include <ctime>
#include <nlohmann/json_fwd.hpp> // IWYU pragma: export
#include <set>
#include <string>
#include <utility>
#include <vector>

class VocableProgress
{
public:
    struct Init
    {
        float easeFactor = 0.F;
        float intervalDay = 0.F;
        std::vector<CardId> triggeredBy;
        std::time_t lastSeen{std::time(nullptr)};
    };

    static constexpr Init new_vocable = {
            .easeFactor = 0.F,
            .intervalDay = 0.F,
            .triggeredBy = {},
            .lastSeen = {}};

    VocableProgress(Init init)
        : easeFactor{init.easeFactor}
        , intervalDay{init.intervalDay}
        , triggerCards{std::move(init.triggeredBy)}
        , lastSeen{init.lastSeen}
    {}

    VocableProgress()
        : VocableProgress(Init{}) {}

    VocableProgress(std::string_view sv);
    [[nodiscard]] auto serialize() const -> std::string;

    struct RepeatRange
    {
        int daysMin;
        int daysNormal;
        int daysMax;
        [[nodiscard]] auto implies(const RepeatRange&) const -> bool;
        auto operator<=>(const RepeatRange&) const -> std::weak_ordering;
    };

    static constexpr int pause_time_minutes = 5;
    using pair_t = std::pair<VocableId, VocableProgress>;
    static constexpr std::string s_id = "id";
    static constexpr std::string s_ease_factor = "ease_factor";
    static constexpr std::string s_interval_day = "interval_day";
    static constexpr std::string s_last_seen = "last_seen";
    static constexpr std::string s_triggered_by = "triggered_by";

    void advanceByEase(const Ease&);
    void triggeredBy(CardId cardId);
    [[nodiscard]] auto getNextTriggerCard(std::set<CardId> availableCardIds) const -> CardId;
    [[nodiscard]] auto recency() const -> float;
    [[nodiscard]] auto pauseTimeOver() const -> bool;
    [[nodiscard]] auto isToBeRepeatedToday() const -> bool;
    [[nodiscard]] auto isAgainVocable() const -> bool;
    [[nodiscard]] auto getRepeatRange() const -> RepeatRange;

    [[nodiscard]] auto IntervalDay() const -> float { return intervalDay; }

    [[nodiscard]] auto EaseFactor() const -> float { return easeFactor; }

    [[nodiscard]] auto dueDays() const -> int;

    static auto toJson(const pair_t&) -> nlohmann::json;
    static auto fromJson(const nlohmann::json&) -> pair_t;

private:
    float easeFactor = 0.F;
    float intervalDay = 0.F;
    std::vector<CardId> triggerCards;
    std::time_t lastSeen{std::time(nullptr)};
};
