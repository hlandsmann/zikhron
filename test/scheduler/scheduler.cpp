#include "scheduler.h"

#include "SRS_data.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <map>
#include <ranges>
#include <tuple>
#include <vector>
namespace ranges = std::ranges;
namespace views = std::ranges::views;

Weight::Weight(int _skewMin, int _skewMax)
    : skewMin{_skewMin}
    , skewMax{_skewMax}
{}

void Weight::adapt(int skew, bool event, int /* failCount */)
{
    if (skew != std::clamp(skew, skewMin, skewMax)) {
        return;
    }
    if (!average.contains(skew)) {
        average[skew] = {
                {.pass = 0, .fail = 0, .deviation = 0.3, .rate = 10},
                {.pass = 0, .fail = 0, .deviation = 0.2, .rate = 20},
                {.pass = 0, .fail = 0, .deviation = 0.1, .rate = 30},
                {.pass = 0, .fail = 0, .deviation = 0.02, .rate = 100},
        };
    }
    auto& avgs = average.at(skew);
    std::vector<double> outcomes;

    for (auto& avg : avgs) {
        outcomes.push_back(avg.adapt(event));
    }

    if (skew != active) {
        return;
    }
    for (const auto& [outcome, avg] : views::reverse(views::zip(outcomes, avgs))) {
        if (outcome != 0) {
            active += static_cast<int>(outcome);
            active = std::clamp(active, skewMin, skewMax);
            // avg.log();
            avg.reset();
            // for (auto& avg_ : avgs) {
            //     avg_.reset();
            // }
            break;
        }
    }
}

auto Weight::get(int /* failCount */) const -> int
{
    return active;
}

auto Weight::Average::adapt(bool event) -> double
{
    int outcome = 0;
    if (event) {
        pass++;
    } else {
        fail++;
    }
    if (fail + pass > rate) {
        double sum = fail + pass;
        double avg = pass / sum;
        if (std::abs(avg - probability) < deviation) {
            // spdlog::info("prob: {}", pass / rate);
        } else if (avg < probability) {
            outcome = -1;
        } else {
            outcome = +1;
        }
        fail = (1 - avg) * (rate - 1);
        pass = avg * (rate - 1);
    }
    return outcome;
}

void Weight::Average::reset()
{
    fail = 0;
    pass = 0;
}

void Weight::Average::log() const
{
    spdlog::info("prob: {:.2f}  --- rate:{}", pass / (pass + fail), rate);
}

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
    int nextInterval = 0;
    if (pass) {
        auto& weight = getPassWeight(lastInterval);
        int skew = weight.get(item.failCount);
        nextInterval = nextIntervalDays(item.interval_0, lastInterval, skew);
        item.interval_0 = lastInterval;
    } else {
        item.interval_0 = lastInterval; // static_cast<int>(std::max(std::log(lastInterval), 1.));
        auto logVal = static_cast<int>(std::max(std::log(lastInterval) / 2, 1.));
        auto& weight = getFailWeight(item.interval_0);
        int skew = weight.get(item.failCount);
        nextInterval = nextIntervalDays(0, logVal, skew);
        // nextInterval =  item.interval_0;
    }
    item.pass = pass;
    item.dueDay = currentDay + nextInterval;
    item.lastDay = currentDay;
    return nextInterval;
}

auto Scheduler::getWeight(int lastInterval, std::map<std::size_t, Weight>& weights) -> Weight&
{
    int a = 1;
    int b = 2;
    std::size_t weightIndex = 0;
    while (a < lastInterval) {
        std::tie(a, b) = std::tuple(b, a + b);
        weightIndex++;
    }
    if (weights.contains(weightIndex)) {
        return weights.at(weightIndex);
    }
    int maxInterval = b - 1;
    int newSkewMin = std::max(-maxInterval, skewMin);
    int newSkewMax = std::min(maxInterval, skewMax);
    weights[weightIndex] = Weight{newSkewMin, newSkewMax};
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
        return 0;
    }

    int skew = interval - (2 * lastInterval);
    if (std::clamp(skew, skewMin, skewMax) == skew) {
        return skew;
    }

    skew = static_cast<int>(
            std::round(
                    10.0 * ((static_cast<double>(interval) / lastInterval) - defaultFactor)));
    return std::clamp(skew, skewMin, skewMax);
}
