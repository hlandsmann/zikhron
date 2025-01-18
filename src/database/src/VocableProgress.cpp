#include <VocableProgress.h>
#include <annotation/Ease.h>
#include <misc/Identifier.h>
#include <utils/Time.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <algorithm>
#include <cmath>
#include <compare>
#include <ctime>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>
namespace ranges = std::ranges;

VocableProgress::VocableProgress(std::string_view sv)
{
    deserialize(sv);
}

auto VocableProgress::serialize() const -> std::string
{
    return fmt::format("{},{:.2F},{:.1F},{},",
                       utl::serialize_time_t(lastSeen),
                       easeFactor,
                       intervalDay,
                       fmt::join(triggerCardIndices, ","));
}

void VocableProgress::deserialize(std::string_view sv)
{
    lastSeen = utl::deserialize_time_t(std::string{utl::split_front(sv, ',')});
    easeFactor = std::stof(std::string{utl::split_front(sv, ',')});
    intervalDay = std::stof(std::string{utl::split_front(sv, ',')});
    while (true) {
        auto cardIndex = std::string{utl::split_front(sv, ',')};
        if (cardIndex.empty()) {
            break;
        }
        triggerCardIndices.push_back(static_cast<CardId>(std::stoul(cardIndex)));
    }
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

auto VocableProgress::triggerValue(CardId cardId, const std::vector<CardId>& availableCardIds) -> std::size_t
{
    auto cardIdit = ranges::find(availableCardIds, cardId);
    auto cardIdindex = static_cast<std::size_t>(std::distance(availableCardIds.begin(), cardIdit));
    auto it = ranges::find(triggerCardIndices, cardIdindex);
    if (it != triggerCardIndices.end()) {
        return triggerCardIndices.size();
    }
    return static_cast<std::size_t>(std::distance(triggerCardIndices.begin(), it));
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
    auto result = (easeFactor * intervalDay) - static_cast<float>(utl::daysFromToday(lastSeen, 0));
    if (dueDays() > 0) {
        result += 1024; // magic number ... just use any high number to offset recency
    }
    return result;
}

auto VocableProgress::pauseTimeOver() const -> bool
{
    std::tm last = *std::localtime(&lastSeen);
    last.tm_min += pause_time_minutes;
    std::time_t last_time = std::mktime(&last);
    std::time_t now_time = std::time(nullptr);

    return last_time < now_time;
}

auto VocableProgress::isNewVocable() const -> bool
{
    return lastSeen == 0;
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
    float daysMinAtleast = std::clamp(intervalDay, 0.F, 1.F);
    return {.daysMin = utl::daysFromToday(lastSeen,
                                          std::max(daysMinAtleast, intervalDay * minFactor)),
            .daysNormal = dueDays(),
            .daysMax = utl::daysFromToday(lastSeen, intervalDay * maxFactor)};
}

auto VocableProgress::dueDays() const -> int
{
    return utl::daysFromToday(lastSeen, intervalDay);
}

auto VocableProgress::getLastSeenStr() const -> std::string
{
    return utl::serialize_time_t(lastSeen);
}

auto VocableProgress::isEnabled() const -> bool
{
    return enabled;
}
