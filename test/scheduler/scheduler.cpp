#include "scheduler.h"

#include "SRS_data.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <map>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>
namespace ranges = std::ranges;
namespace views = std::ranges::views;


auto Scheduler::nextInterval(int currentDay, int id, bool pass) -> int
{
    if (!items.contains(id)) {
        items[id] = SRS_data{};
        items[id].dueDay = currentDay + 1;
        items[id].lastDay = currentDay;
        items[id].pass = pass;
        return 1;
    }
    auto& item = items[id];
    auto& oldWeight = item.pass ? getPassWeight(item.interval_0) : getFailWeight(item.interval_0);
    int lastInterval = std::min(item.dueDay, currentDay) - item.lastDay;
    int oldSkew = getSkewFromInterval(item.interval_0, lastInterval);
    if (!item.pass) {
        oldSkew = getSkewFromInterval(std::max(std::log(item.interval_0) / 2, 1.), lastInterval);
    }
    oldWeight.adapt(oldSkew, pass, item.failCount);
    // if (!pass) {
    //     spdlog::info("fail: i0: {}, li: {}, oskew: {}", item.interval_0, lastInterval, oldSkew);
    // } else {
    //     spdlog::info("pass: i0: {}, li: {}, oskew: {}", item.interval_0, lastInterval, oldSkew);
    // }
    int nextInterval = 0;
    if (pass) {
        auto& weight = getPassWeight(lastInterval);
        int skew = weight.get(item.failCount);
        nextInterval = nextIntervalDays(item.interval_0, lastInterval, skew);
        // spdlog::info("i0: {}, li: {}, ni: {}", item.interval_0, lastInterval, nextInterval);
        item.interval_1 = item.interval_0;
        item.interval_0 = lastInterval;

        // auto oskew = getSkewFromInterval(lastInterval, nextInterval);
        // if (oskew != skew) {
        //     spdlog::info("lastInterval: {}, nextInterval: {}, oskew: {}, skew: {} --- {}", lastInterval, nextInterval, oskew, skew, weight.getFmt());
        // }
        // assert(oskew == skew);
    } else {
        item.failCount++;
        auto& weight = getFailWeight(lastInterval);
        auto logVal = static_cast<int>(std::max(std::log(lastInterval) / 2, 1.));
        int skew = weight.get(item.failCount);
        nextInterval = nextIntervalDays(0, logVal, skew);
        item.interval_0 = lastInterval; // static_cast<int>(std::max(std::log(lastInterval), 1.));
        // nextInterval =  item.interval_0;
    }
    item.pass = pass;
    item.dueDay = currentDay + nextInterval;
    item.lastDay = currentDay;
    return nextInterval;
}

void Scheduler::rescheduleInternal()
{
    for (auto& [_, item] : items) {
        int nextInterval = 0;
        int lastInterval = item.interval_0;
        if (item.pass) {
            auto& weight = getPassWeight(lastInterval);
            int skew = weight.get(item.failCount);
            nextInterval = nextIntervalDays(item.interval_1, lastInterval, skew);
        } else {
            auto& weight = getFailWeight(lastInterval);
            auto logVal = static_cast<int>(std::max(std::log(lastInterval) / 2, 1.));
            int skew = weight.get(item.failCount);
            nextInterval = nextIntervalDays(0, logVal, skew);
        }
        item.dueDay = item.lastDay + nextInterval;
    }
}

auto Scheduler::getNextReview(int id) -> int
{
    const auto& item = items.at(id);
    return item.dueDay;
}

auto Scheduler::shouldReschedule() -> bool
{
    bool result = false;
    for (auto& [_, weight] : passWeights) {
        result |= weight.wasChanged();
    }
    for (auto& [_, weight] : failWeights) {
        result |= weight.wasChanged();
    }
    return result;
}

auto Scheduler::getWeight(int lastInterval, std::map<std::size_t, Weight>& weights) -> Weight&
{
    std::size_t weightIndex = 0;
    if (lastInterval == 0) {
        if (weights.contains(weightIndex)) {
            return weights.at(weightIndex);
        }
        weights[weightIndex] = Weight{0, 0, 0, 2};
        return weights.at(weightIndex);
    }
    weightIndex++;
    int a = 1;
    int b = 2;
    while (a < lastInterval) {
        std::tie(a, b) = std::tuple(b, a + b);
        weightIndex++;
    }
    if (weights.contains(weightIndex)) {
        return weights.at(weightIndex);
    }
    int minInterval = a;
    int newSkewMin = std::max(-minInterval, skewMin);
    int newSkewMax = std::min(minInterval, skewMax);
    weights[weightIndex] = Weight{minInterval, b - 1, newSkewMin, newSkewMax};
    return weights.at(weightIndex);

    // std::size_t weightIndex = getWeightIndex(static_cast<std::uint32_t>(lastInterval));
    // if (weights.contains(weightIndex)) {
    //     return weights.at(weightIndex);
    // }
    //
    // int maxInterval = (2 << weightIndex) - 1;
    // int newSkewMin = std::max(-maxInterval, skewMin);
    // int newSkewMax = std::min(maxInterval, skewMax);
    // weights[weightIndex] = Weight{newSkewMin, newSkewMax};
    // return weights.at(weightIndex);
}

auto Scheduler::getFailWeight(int lastInterval) -> Weight&
{
    return getWeight(lastInterval, failWeights);
}

auto Scheduler::getPassWeight(int lastInterval) -> Weight&
{
    return getWeight(lastInterval, passWeights);
}

auto Scheduler::getWeightIndex(std::uint32_t interval) -> std::size_t
{
    auto result = std::max(0, std::bit_width(interval) - 1);
    return static_cast<std::size_t>(result);
}

auto Scheduler::nextIntervalDays(int lastInterval, int skew) -> int
{
    if (skew < skewMin || skew > skewMax) {
        return 0;
    }
    if (lastInterval == 0) {
        return std::clamp(skew, 1, 3);
    }

    int newInterval = (lastInterval * defaultFactor) + skew;
    double factor = defaultFactor + (easeStep * skew);
    double dInterval = lastInterval * factor;

    if (skew < 0) {
        if (newInterval < lastInterval) {
            return lastInterval;
        }
        return std::min(newInterval, static_cast<int>(dInterval));
    }
    if (skew > 0) {
        if (newInterval > lastInterval * 3) {
            return lastInterval * 3;
        }
        return std::max(newInterval, static_cast<int>(dInterval));
    }
    return newInterval;
}

auto Scheduler::nextIntervalDays(int lastStableInterval, int lastInterval, int skew) -> int
{
    int newInterval = nextIntervalDays(lastInterval, skew);
    if (newInterval == lastStableInterval) {
        newInterval++;
    }

    return newInterval;
}

auto Scheduler::getSkewFromInterval(int lastInterval, int interval) -> int
{
    if (lastInterval <= 0 || interval < 0) {
        return std::clamp(interval, 1, 3);
    }

    int skew = interval - (2 * lastInterval);
    if (nextIntervalDays(lastInterval, skew) == interval) {
        return skew;
    }
    // if (std::clamp(skew, skewMin, skewMax) == skew) {
    //     return skew;
    // }
    skew = static_cast<int>(
            std::floor(
                    10.0 * ((static_cast<double>(interval) / lastInterval) - defaultFactor)));

    skew = std::clamp(skew, skewMin, skewMax);
    if (nextIntervalDays(lastInterval, skew) == interval) {
        return skew;
    }
    return std::clamp(skew + 1, skewMin, skewMax);
}

void Scheduler::test()
{
    spdlog::info("Test");

    for (int skew = -7; skew <= 10; skew++) {
        int oldInterval = 0;
        int nextInterval = nextIntervalDays(oldInterval, skew);
        int oldSkew = getSkewFromInterval(oldInterval, nextInterval);
        spdlog::info("oi: {}, ni: {}, skew: {}, oskew: {}", oldInterval, nextInterval, skew, oldSkew);
    }
    spdlog::info("-----------");
    for (int skew = -7; skew <= 10; skew++) {
        int oldInterval = 45;
        int nextInterval = nextIntervalDays(oldInterval, skew);
        int oldSkew = getSkewFromInterval(oldInterval, nextInterval);
        spdlog::info("oi: {}, ni: {}, skew: {}, oskew: {}", oldInterval, nextInterval, skew, oldSkew);
    }
}
