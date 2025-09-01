#pragma once
#include "SRS_data.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

class Weight
{
public:
    Weight() = default;
    Weight(int skewMin, int skewMax);
    void adapt(int skew, bool event, int failCount);
    [[nodiscard]] auto get(int failCount) const -> int;

private:
    int skewMin{};
    int skewMax{};

    struct Average
    {
        static constexpr double probability = 0.9;
        double pass{};
        double fail{};
        double deviation{};
        double rate{};

        auto adapt(bool event) -> double;
        void reset();

        void log() const;
    };

    int active{};

    std::map<int, std::vector<Average>> average;
};

class Scheduler
{
public:
    auto nextInterval(int currentDay, int id, bool pass) -> int;

    void clearItems() { items.clear(); }

    void printWeights()
    {
        for (const auto& [w, s] : passWeights) {
            spdlog::info("w({}) : {}", w, s.get(0));
        }
        fmt::print("\n");

        for (const auto& [w, s] : failWeights) {
            spdlog::info("w({}) : {}", w, s.get(0));
        }
    }

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
