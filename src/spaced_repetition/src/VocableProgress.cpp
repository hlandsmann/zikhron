#include <VocableProgress.h>
#include <annotation/Ease.h>
#include <fmt/format.h>
#include <misc/Identifier.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <algorithm>
#include <cmath>
#include <compare>
#include <ctime>
#include <iterator>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <string_view>
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

VocableProgress::VocableProgress(std::string_view sv)
{
    lastSeen = deserialize_time_t(std::string{utl::split_front(sv, ',')});
    easeFactor = std::stof(std::string{utl::split_front(sv, ',')});
    intervalDay = std::stof(std::string{utl::split_front(sv, ',')});
    while (true) {
        auto cardId = std::string{utl::split_front(sv, ',')};
        if (cardId.empty()) {
            break;
        }
        triggerCardIndices.push_back(static_cast<CardId>(std::stoul(cardId)));
    }
}

auto VocableProgress::serialize() const -> std::string
{
    return fmt::format("{},{:.2F},{:.1F},{},",
                       serialize_time_t(lastSeen),
                       easeFactor,
                       intervalDay,
                       fmt::join(triggerCardIndices, ","));
}

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

void VocableProgress::triggeredBy(CardId cardId, const std::vector<CardId>& availableCardIds)
{
    auto cardIdit = ranges::find(availableCardIds, cardId);
    auto cardIdindex = static_cast<std::size_t>(std::distance(availableCardIds.begin(), cardIdit));
    auto it = ranges::find(triggerCardIndices, cardIdindex);
    if (it != triggerCardIndices.end()) {
        triggerCardIndices.erase(it);
    }
    triggerCardIndices.push_back(cardIdindex);
}

auto VocableProgress::getNextTriggerCard(const std::vector<CardId>& availableCardIds) const -> CardId
{
    std::vector<std::size_t> triggerCardsTemp;
    ranges::copy_if(triggerCardIndices, std::back_inserter(triggerCardsTemp),
                    [maxIndex = availableCardIds.size() - 1](std::size_t index) -> bool {
                        return index <= maxIndex;
                    });

    std::vector<CardId> triggerCardIds;
    ranges::transform(triggerCardsTemp, std::back_inserter(triggerCardIds),
                      [&availableCardIds](std::size_t index) { return availableCardIds.at(index); });
    CardId result{};
    if (triggerCardsTemp.size() == availableCardIds.size()) {
        result = availableCardIds.at(triggerCardsTemp.front());
        spdlog::info("Triggered so far by [{}], now triggering: {} (old card)", fmt::join(triggerCardIds, ", "), result);
        return result;
    }
    ranges::sort(triggerCardsTemp);
    std::size_t cardIndex = 0;
    for (auto index : triggerCardsTemp) {
        if (cardIndex != index) {
            break;
        }
        cardIndex++;
    }
    result = availableCardIds.at(cardIndex);
    spdlog::info("Triggered so far by [{}], now triggering: {} (new avaliable)", fmt::join(triggerCardIds, ", "), result);
    return result;
}

auto VocableProgress::recency() const -> float
{
    return (easeFactor * intervalDay) - static_cast<float>(daysFromToday(lastSeen, 0));
}

auto VocableProgress::pauseTimeOver() const -> bool
{
    std::tm last = *std::localtime(&lastSeen);
    last.tm_min += pause_time_minutes;
    std::time_t last_time = std::mktime(&last);
    std::time_t now_time = std::time(nullptr);

    return last_time < now_time;
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
    return {.daysMin = daysFromToday(lastSeen,
                                     std::max(daysMinAtleast, intervalDay * minFactor)),
            .daysNormal = dueDays(),
            .daysMax = daysFromToday(lastSeen, intervalDay * maxFactor)};
}

auto VocableProgress::dueDays() const -> int
{
    return daysFromToday(lastSeen, intervalDay);
}
