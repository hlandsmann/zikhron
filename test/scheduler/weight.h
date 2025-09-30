#pragma once
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

class Weight2
{
public:
    Weight2() = default;
    Weight2(int valMin, int valMax, int skewMin, int skewMax);
    void adapt(int skew, bool event, int failCount);
    [[nodiscard]] auto get(int failCount) const -> int;
    [[nodiscard]] auto getFmt() const -> std::string;
    [[nodiscard]] auto wasChanged() -> bool;

private:
    int valMin{};
    int valMax{};
    int skewMin{};
    int skewMax{};
    bool changed{false};

    int adaptCount{};
    int silentCount{};
    int changeCount{};

    struct Average
    {
        static constexpr double probability = 0.9;

        void adapt(bool event, int failCount);
        [[nodiscard]] auto get(int count, double deviation, int rate) const -> int;
        [[nodiscard]] auto getProbability(int count, int rate) const -> double;
        void reset(int rate);

        void log() const;
        std::map<int, std::vector<bool>> count_outcome;
        std::map<int, int> rateCounter;
    };

    std::map<int, int> count_active;

    std::map<int, Average> skew_average;

    struct Adaption
    {
        double deviation{};
        int rate{};
    };
};
