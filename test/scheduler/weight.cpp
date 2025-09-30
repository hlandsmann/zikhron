#include "weight.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <generator>
#include <map>
#include <ranges>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
namespace ranges = std::ranges;
namespace views = std::ranges::views;

Weight2::Weight2(int _valMin, int _valMax, int _skewMin, int _skewMax)
    : valMin{_valMin}
    , valMax{_valMax}
    , skewMin{_skewMin}
    , skewMax{_skewMax}
{}

void Weight2::adapt(int skew, bool event, int failCount)
{
    constexpr std::array<Adaption, 4> adaption = {
            Adaption{.deviation = 0.3, .rate = 10},
            {.deviation = 0.2, .rate = 20},
            {.deviation = 0.1, .rate = 30},
            {.deviation = 0.02, .rate = 100},
    };

    if (skew != std::clamp(skew, skewMin, skewMax)) {
        return;
    }
    auto& avg = skew_average[skew];
    avg.adapt(event, failCount);
    for (const auto& [deviation, rate] : adaption) {
        avg.rateCounter[rate]++;
    }

    auto active = get(failCount);
    if (active != skew) {
        silentCount++;
        return;
    }
    adaptCount++;
    for (const auto& [deviation, rate] : adaption) {
        if (auto res = avg.get(failCount, deviation, rate); res != 0) {
            count_active[failCount] = std::clamp(skew + res, skewMin, skewMax);
            avg.reset(rate);
            changed = true;
            changeCount++;
        }
    }
}

auto Weight2::get(int failCount) const -> int
{
    if (count_active.empty()) {
        return 0;
    }
    const auto& [_, skew] = *ranges::min_element(count_active, std::less{}, [failCount](const auto& count_skew) -> int {
        const auto& [count, _] = count_skew;
        return std::abs(failCount - count);
    });
    return skew;
}

void Weight2::Average::adapt(bool event, int failCount)
{
    auto& outcome = count_outcome[failCount];
    outcome.push_back(event);
}

auto nextEvent(int count, const std::map<int, std::vector<bool>>& outcome) -> std::generator<bool>
{
    if (outcome.empty()) {
        co_return;
    }
    auto it_1 = outcome.begin();
    auto it_2 = outcome.begin();
    for (; it_2 != outcome.end(); it_2++) {
        const auto& [c2, _] = *it_2;
        if (c2 < count) {
            it_1 = it_2;
        } else {
            break;
        }
    }

    if (it_1 == it_2) {
        it_2++;
    }

    bool beginEmptied = false;
    // spdlog::info("ne, size: {}", outcome.size());
    while (true) {
        const auto& [c1, _] = *it_1;
        // spdlog::info("loop");

        auto it = it_1;
        assert(it_1 != it_2);
        if (it_2 != outcome.end()) {
            const auto& [c2, _] = *it_2;
            // assert(c2 - count >= 0);
            // assert(count - c1 >= 0);
            if (c2 - count < count - c1) {
                it = it_2;
                it_2++;
            }
        } else if (beginEmptied) {
            break;
        }
        const auto& [c, outc] = *it;
        for (bool event : views::reverse(outc)) {
            co_yield event;
        }
        if (it == it_1) {
            if (it_1 == outcome.begin()) {
                beginEmptied = true;
            } else {
                it_1--;
            }
        }
    }
    co_yield true;
}

auto Weight2::Average::get(int count, double deviation, int rate) const -> int
{
    if (rateCounter.contains(rate) && rateCounter.at(rate) < rate) {
        return 0;
    }
    double prob = getProbability(count, rate);
    if (std::abs(prob - probability) < deviation) {
        // spdlog::info("prob: {}", pass / rate);
    } else if (prob < probability) {
        return -1;
    } else {
        return +1;
    }
    return 0;
}

auto Weight2::Average::getProbability(int count, int rate) const -> double
{
    double pass = 0;
    double fail = 0;
    int rateCount = 0;

    for (bool event : nextEvent(count, count_outcome)) {
        if (event) {
            pass++;
        } else {
            fail++;
        }
        if (++rateCount == rate) {
            break;
        }
    }
    double sum = fail + pass;
    double prob = pass / sum;
    return prob;
}

void Weight2::Average::reset(int rate)
{
    constexpr std::array<std::pair<int, int>, 4> rateReset = {{{10, 0}, {20, 5}, {30, 10}, {100, 60}}};
    for (const auto [upperRate, resetTo] : rateReset) {
        if (upperRate >= rate) {
            rateCounter[rate] = resetTo;
        }
    }
}

void Weight2::Average::log() const
{
}

auto Weight2::getFmt() const -> std::string
{
    if (count_active.empty()) {
        return {};
    }
    // auto [count, outcome] = *count_active.begin();
    int count = -1;
    int outcome = -1;
    std::string result = fmt::format("[{} - {}] - a: {}, s: {}, c: {}", valMin, valMax, adaptCount, silentCount, changeCount);
    for (const auto& [countTmp, outcomeTmp] : count_active) {
        if (outcomeTmp != outcome) {
            count = countTmp;
            outcome = outcomeTmp;
            if (!skew_average.contains(outcome)) {
                result = fmt::format("{}, [{}:{} --- ]", result, count, outcome);

            } else {
                const auto& avg = skew_average.at(outcome);
                double prob = avg.getProbability(count, 100);

                result = fmt::format("{}, [{}:{} {:.2f}]", result, count, outcome, prob);
            }
        }
    }
    return result;
}

auto Weight2::wasChanged() -> bool
{
    if (changed) {
        changed = false;
        return true;
    }
    return false;
}
