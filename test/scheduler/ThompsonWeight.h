#pragma once
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <map>
#include <random>
#include <ranges>
#include <string>
#include <vector>

// struct BetaState
// {
//     double alpha{1.0};
//     double beta{0.0};
//     double monteCarlo{0.0};
//     auto operator==(const BetaState& bt) const -> bool = default;
// };

class ThompsonWeight
{
public:
    ThompsonWeight() = default;
    ThompsonWeight(int valMin, int valMax, int skewMin, int skewMax);
    void adapt(int skew, bool event, int failCount);
    [[nodiscard]] auto get(int failCount) -> int;
    [[nodiscard]] auto getFmt() const -> std::string;
    [[nodiscard]] auto wasChanged() -> bool;
    void train();

private:
    struct BetaState
    {
        double alpha{1.0};
        double beta{0.0};
        double monteCarlo{0.0};
        auto operator==(const BetaState& bt) const -> bool = default;
    };

    auto getStates(int failCount) -> std::vector<BetaState>&;

    [[nodiscard]] auto getBestIndex(const std::vector<BetaState>& states) const -> std::size_t;

    template<std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, BetaState>
    auto getSampleIndex(R rangeStates) const -> std::size_t;

    void monteCarlo(std::vector<BetaState>& states);
    static auto getPropabiltiy(const BetaState& weight) -> double;
    void setup();
    void createBetaStates();

    // -----------------
    mutable std::mt19937 gen;

    struct Event
    {
        std::size_t index;
        bool event;
    };

    std::vector<std::vector<BetaState>> difficultyStates;
    std::map<int, std::vector<Event>> failCount_events;
    std::vector<int> mergers;

    int valMin{};
    int valMax{};
    int skewMin{};
    int skewMax{};

    // std::vector<BetaState> states;
};
