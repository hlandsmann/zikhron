// #pragma once
// #include <annotation/Ease.h>
// #include <misc/Identifier.h>
//
// #include <compare>
// #include <ctime>
// #include <string>
// #include <string_view>
// #include <utility>
// #include <vector>
//
// class VocableProgress
// {
// public:
//     struct Init
//     {
//         float easeFactor = 0.F;
//         float intervalDay = 0.F;
//         std::vector<std::size_t> triggeredBy;
//         std::time_t lastSeen{std::time(nullptr)};
//         bool enabled = false;
//     };
//
//     static constexpr Init new_vocable = {
//             .easeFactor = 0.F,
//             .intervalDay = 0.F,
//             .triggeredBy = {},
//             .lastSeen = {},
//             .enabled = {}};
//
//     VocableProgress(Init init)
//         : easeFactor{init.easeFactor}
//         , intervalDay{init.intervalDay}
//         , triggerCardIndices{std::move(init.triggeredBy)}
//         , lastSeen{init.lastSeen}
//         , enabled{init.enabled}
//     {}
//
//     VocableProgress()
//         : VocableProgress(Init{}) {}
//
//     VocableProgress(std::string_view sv);
//     [[nodiscard]] auto serialize() const -> std::string;
//
//
//     static constexpr int pause_time_minutes = 5;
//     using pair_t = std::pair<VocableId, VocableProgress>;
//     static constexpr std::string s_id = "id";
//     static constexpr std::string s_ease_factor = "ease_factor";
//     static constexpr std::string s_interval_day = "interval_day";
//     static constexpr std::string s_last_seen = "last_seen";
//     static constexpr std::string s_triggered_by = "triggered_by";
//
//     void advanceByEase(const Ease&);
//     void triggeredBy(CardId cardId, const std::vector<CardId>& availableCardIds);
//     auto triggerValue(CardId cardId, const std::vector<CardId>& availableCardIds) -> std::size_t;
//     [[nodiscard]] auto getNextTriggerCard(const std::vector<CardId>& availableCardIds) const -> CardId;
//     [[nodiscard]] auto recency() const -> float;
//     [[nodiscard]] auto pauseTimeOver() const -> bool;
//     [[nodiscard]] auto isNewVocable() const -> bool;
//     [[nodiscard]] auto isAgainVocable() const -> bool;
//
//     [[nodiscard]] auto IntervalDay() const -> float { return intervalDay; }
//
//     [[nodiscard]] auto EaseFactor() const -> float { return easeFactor; }
//
//     [[nodiscard]] auto dueDays() const -> int;
//     [[nodiscard]] auto getLastSeenStr() const -> std::string;
//
//     [[nodiscard]] auto isEnabled() const -> bool;
//     void setEnabled(bool enabled);
//
//     void deserialize(std::string_view sv);
//     float easeFactor = 0.F;
//     float intervalDay = 0.F;
//     std::vector<std::size_t> triggerCardIndices;
//     std::time_t lastSeen{std::time(nullptr)};
//     bool enabled = false;
// };
