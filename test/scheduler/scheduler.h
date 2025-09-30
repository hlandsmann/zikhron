#pragma once
#include "SRS_data.h"
#include "weight.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

struct History
{
};

class Scheduler
{
public:
    using Weight = Weight2;
    auto nextInterval(int currentDay, int id, bool pass) -> int;
    auto getNextReview(int id) -> int;

    auto shouldReschedule() -> bool;
    void rescheduleInternal();

    void clearItems() { items.clear(); }

    void printWeights()
    {
        for (const auto& [w, s] : passWeights) {
            spdlog::info("w({}) : {}", w, s.getFmt());
        }
        fmt::print("\n");

        for (const auto& [w, s] : failWeights) {
            spdlog::info("w({}) : {}", w, s.getFmt());
        }
    }

    void test();

private:
    static auto getWeight(int lastInterval, std::map<std::size_t, Weight>& weights) -> Weight&;
    auto getFailWeight(int lastInterval) -> Weight&;
    auto getPassWeight(int lastInterval) -> Weight&;

    static auto getWeightIndex(std::uint32_t interval) -> std::size_t;
    static auto nextIntervalDays(int lastInterval, int skew) -> int;
    static auto nextIntervalDays(int lastStableInterval, int lastInterval, int skew) -> int;
    static auto getSkewFromInterval(int lastInterval, int interval) -> int;
    static constexpr int defaultFactor = 2;
    static constexpr double easeStep = 0.1;

    static constexpr int skewMin = -7;
    static constexpr int skewMax = 10;
    std::map<int, SRS_data> items;
    std::map<std::size_t, Weight> passWeights;
    std::map<std::size_t, Weight> failWeights;
};
