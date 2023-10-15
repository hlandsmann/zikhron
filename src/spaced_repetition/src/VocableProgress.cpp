#include <VocableProgress.h>
#include <annotation/Ease.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>
#include <compare>
#include <ctime>
#include <iterator>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "Time.h"
using namespace spaced_repetition;
namespace ranges = std::ranges;
namespace {

template<class T>
auto serialize_vector(std::vector<T> vector) -> nlohmann::json
{
    auto array = nlohmann::json::array();
    ranges::copy(vector, std::back_inserter(array));
    return array;
}

template<class T>
auto deserialize_vector(nlohmann::json json) -> std::vector<T>
{
    std::vector<T> result;
    ranges::copy(json, std::back_inserter(result));
    return result;
}

} // namespace

auto VocableProgress::RepeatRange::implies(const RepeatRange& other) const -> bool
{
    return std::max(0, daysMax) >= other.daysMin;
}

auto VocableProgress::RepeatRange::operator<=>(const RepeatRange& other) const -> std::weak_ordering
{
    if (daysMax < other.daysMax) {
        return std::weak_ordering::less;
    }
    if (daysMax > other.daysMax) {
        return std::weak_ordering::greater;
    }
    if (daysNormal < other.daysNormal) {
        return std::weak_ordering::less;
    }
    if (daysNormal > other.daysNormal) {
        return std::weak_ordering::greater;
    }
    if (daysMin < other.daysMin) {
        return std::weak_ordering::less;
    }
    if (daysMin > other.daysMin) {
        return std::weak_ordering::greater;
    }
    return std::weak_ordering::equivalent;
}

void VocableProgress::advanceByEase(const Ease& ease)
{
    lastSeen = std::time(nullptr);

    auto progress = ease.getProgress();
    intervalDay = progress.intervalDay;
    easeFactor = progress.easeFactor;
}

void VocableProgress::triggeredBy(CardId cardId)
{
    auto it = ranges::find(triggerCards, cardId);
    if (it != triggerCards.end()) {
        triggerCards.erase(it);
    }
    triggerCards.push_back(cardId);
}

auto VocableProgress::recency() const -> float
{
    return (easeFactor * intervalDay) - static_cast<float>(daysFromNow(lastSeen, 0));
}

auto VocableProgress::pauseTimeOver() const -> bool
{
    std::tm last = *std::localtime(&lastSeen);
    last.tm_min += pause_time_minutes;
    std::time_t last_time = std::mktime(&last);
    std::time_t now_time = std::time(nullptr);

    return last_time < now_time;
}

auto VocableProgress::fromJson(const nlohmann::json& jsonIn) -> pair_t
{
    Init init = {.easeFactor = jsonIn.at(VocableProgress::s_ease_factor),
                 .intervalDay = jsonIn.at(VocableProgress::s_interval_day),
                 .triggeredBy = deserialize_vector<CardId>(jsonIn.at(VocableProgress::s_triggered_by)),
                 .lastSeen = deserialize_time_t(jsonIn.at(VocableProgress::s_last_seen))};
    return {jsonIn.at(std::string(VocableProgress::s_id)), {init}};
}

auto VocableProgress::toJson(const pair_t& pair) -> nlohmann::json
{
    const VocableProgress& vocSR = pair.second;
    return {
            {VocableProgress::s_id, pair.first},
            {VocableProgress::s_ease_factor, vocSR.easeFactor},
            {VocableProgress::s_interval_day, vocSR.intervalDay},
            {VocableProgress::s_triggered_by, serialize_vector(vocSR.triggerCards)},
            {VocableProgress::s_last_seen, serialize_time_t(vocSR.lastSeen)}};
}

auto VocableProgress::isToBeRepeatedToday() const -> bool
{
    std::time_t todayMidnight = todayMidnightTime();
    std::time_t vocActiveTime = advanceTimeByDays(lastSeen, intervalDay);

    return todayMidnight > vocActiveTime;
}

auto VocableProgress::isAgainVocable() const -> bool
{
    return intervalDay == 0;
};

auto VocableProgress::getRepeatRange() const -> RepeatRange
{
    constexpr auto square = 2.F;
    float minFactor = std::pow(Ease::changeFactorHard, square);
    float maxFactor = easeFactor * Ease::changeFactorHard;
    float daysMinAtleast = (intervalDay >= 1.F) ? 1.F : 0.F;
    return {.daysMin = daysFromNow(lastSeen,
                                   std::max(daysMinAtleast, intervalDay * minFactor)),
            .daysNormal = dueDays(),
            .daysMax = daysFromNow(lastSeen, intervalDay * maxFactor)};
}

auto VocableProgress::dueDays() const -> int
{
    return daysFromNow(lastSeen, intervalDay);
}
