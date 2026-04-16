#include "ThompsonWeight.h"

#include <spdlog/spdlog.h>
#include <utils/format.h>

#include <algorithm>
#include <boost/math/special_functions/beta.hpp>
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <limits>
#include <random>
#include <ranges>
#include <span>
#include <string>
#include <vector>

namespace ranges = std::ranges;
namespace views = std::ranges::views;

ThompsonWeight::ThompsonWeight(int _valMin, int _valMax, int _skewMin, int _skewMax)
    : gen{std::random_device{}()}
    // , states(static_cast<std::size_t>(1 + _skewMax - _skewMin))
    , valMin{_valMin}
    , valMax{_valMax}
    , skewMin{_skewMin}
    , skewMax{_skewMax}
{
    createBetaStates();
    // spdlog::info("size(): {}", 1 + _skewMin - _skewMax);
    // exit(1);
}

void ThompsonWeight::setup()
{
}

void ThompsonWeight::createBetaStates()
{
    difficultyStates.clear();
    std::generate_n(std::back_inserter(difficultyStates), mergers.size() + 1, [this]() {
        return std::vector<BetaState>(static_cast<std::size_t>(1 + skewMax - skewMin));
    });
    for (const auto& s : difficultyStates) {
        // spdlog::info("size: {}", s.size());
    }

    int topDifficulty = -1;
    auto diffIt = mergers.begin();
    auto statesIt = std::next(difficultyStates.begin(), -1);
    for (const auto& [difficulty, events] : failCount_events) {
        // spdlog::info("diff: {}, events.size: {}", difficulty, events.size());

        if (difficulty > topDifficulty) {
            if (diffIt == mergers.end()) {
                topDifficulty = std::numeric_limits<decltype(topDifficulty)>::max();
            } else {
                topDifficulty = *diffIt;
                diffIt++;
            }
            statesIt++;
        }
        auto& states = *statesIt;
        // spdlog::info("stateIt: {}", std::distance(difficultyStates.begin(), statesIt));
        for (const auto& [index, event] : events) {
            // spdlog::info("index: {}, event: {}", index, event);
            if (event) {
                states.at(index).alpha++;
            } else {
                states.at(index).beta++;
            }
        }
    }
}

auto ThompsonWeight::getStates(int failCount) -> std::vector<BetaState>&
{
    const auto it = ranges::find_last_if(mergers,
                                         [failCount](std::size_t failCountTop) -> bool {
                                             return static_cast<std::size_t>(failCount) <= failCountTop;
                                         });

    auto index = static_cast<std::size_t>(ranges::distance(mergers.begin(), it.begin()));
    return difficultyStates.at(index);
}

void ThompsonWeight::adapt(int skew, bool event, int failCount)
{
    if (skew != std::clamp(skew, skewMin, skewMax)) {
        return;
    }
    auto index = static_cast<std::size_t>(skew - skewMin);

    auto& events = failCount_events[failCount];
    events.push_back({.index = index, .event = event});
}

auto ThompsonWeight::get(int failCount) -> int
{
    auto bestIndex = static_cast<int>(
            getBestIndex(
                    getStates(failCount)));
    return bestIndex + skewMin;
    return 0;
}

auto ThompsonWeight::getFmt() const -> std::string
{
    std::string result = fmt::format("min: {}, max: {}\n", valMin, valMax);

    for (const auto& states : difficultyStates) {
        auto startIndex = -skewMin;
        constexpr double target = 0.9;
        // monteCarlo();
        auto itPivot = ranges::find_if(states, [](const BetaState& weight) -> bool { return getPropabiltiy(weight) < target; });
        itPivot = std::max(itPivot, states.begin() + 1);
        if (itPivot == states.end()) {
            itPivot = ranges::min_element(states, ranges::less{}, &getPropabiltiy);
            itPivot = std::max(std::next(states.begin(), startIndex), itPivot);
        } else {
            auto itPrevious = std::next(itPivot, -1);
            itPrevious = std::max(states.begin(), itPrevious);
            if (itPrevious->monteCarlo > itPivot->monteCarlo) {
                itPivot = itPrevious;
            }
        }
        auto pivot = std::distance(states.begin(), itPivot);
        result += fmt::format("{}: ", pivot);
        for (const auto& [index, state] : views::enumerate(states)) {
            if (index < pivot - 2 || index > pivot + 2) {
                continue;
            }
            if (index == pivot) {
                result += "P";
            } else {
                result += " ";
            }
            result += fmt::format("[{:.3f}, {:.3f}, {:4d}]",
                                  getPropabiltiy(state), state.monteCarlo, static_cast<int>(state.alpha + state.beta));
        }
        std::size_t sum = 0;
        for (const auto& [d, e] : failCount_events) {
            sum += e.size();
        }
        result += fmt::format(" s: {}\n", sum);
    }
    return result;
}

auto ThompsonWeight::wasChanged() -> bool
{
    return false;
}

void ThompsonWeight::train()
{
    mergers.clear();
    std::size_t sum = 0;
    for (const auto& [d, e] : failCount_events) {
        sum += e.size();
    }

    auto ul = std::max<std::size_t>(200, sum / 10);
    sum = 0;
    for (const auto& [d, e] : failCount_events) {
        sum += e.size();
        if (sum > ul) {
            mergers.push_back(d);
            sum = 0;
        }
    }

    createBetaStates();
    for (auto& states : difficultyStates) {
        monteCarlo(states);
    }
}

auto ThompsonWeight::getPropabiltiy(const BetaState& weight) -> double
{
    return weight.alpha / (weight.alpha + weight.beta);
}

auto ThompsonWeight::getBestIndex(const std::vector<BetaState>& states) const -> std::size_t
{
    auto startIndex = -skewMin;
    constexpr double target = 0.9;
    // monteCarlo();
    auto itPivot = ranges::find_if(states, [](const BetaState& weight) -> bool { return getPropabiltiy(weight) < target; });
    itPivot = std::max(itPivot, states.begin() + 1);
    if (itPivot == states.end()) {
        itPivot = ranges::min_element(states, ranges::less{}, &getPropabiltiy);
        itPivot = std::max(std::next(states.begin(), startIndex), itPivot);
    } else {
        auto itPrevious = std::next(itPivot, -1);
        itPrevious = std::max(states.begin(), itPrevious);
        if (itPrevious->monteCarlo > itPivot->monteCarlo) {
            itPivot = itPrevious;
        }
    }
    auto itBegin = std::max(states.begin(), std::next(itPivot, -1));
    auto itEnd = std::min(states.end(), std::next(itBegin, 3));
    auto distanceBegin = std::distance(states.begin(), itBegin);
    return getSampleIndex(ranges::subrange{itBegin, itEnd})
           + static_cast<std::size_t>(distanceBegin);
}

template<std::ranges::input_range R>
    requires std::same_as<std::ranges::range_value_t<R>, ThompsonWeight::BetaState>
auto ThompsonWeight::getSampleIndex(R rangeStates) const -> std::size_t
{
    constexpr double target = 0.9;
    std::size_t best_index = 0;
    double min_dist = 2.0; // Max possible distance is 1.0

    for (const auto& [index, weight] : views::enumerate(rangeStates)) {
        // Generate a random sample from Beta(alpha, beta)
        std::gamma_distribution<double> dist_a(weight.alpha + 1, 1.0);
        std::gamma_distribution<double> dist_b(weight.beta + 1, 1.0);

        double a = dist_a(gen);
        double b = dist_b(gen);
        double beta_sample = a / (a + b);

        double dist = std::abs(beta_sample - target);
        if (dist < min_dist) {
            min_dist = dist;
            best_index = static_cast<std::size_t>(index);
        }
    }
    return best_index;
}

void ThompsonWeight::monteCarlo(std::vector<BetaState>& states)
{
    std::vector<double> result(states.size());
    constexpr double runs = 1000;
    for (int i = 0; i < runs; i++) {
        auto index = getSampleIndex(states);
        result.at(index)++;
    }
    for (const auto& [w, r] : views::zip(states, result)) {
        r = r / runs;
        w.monteCarlo = r;
    }
}
