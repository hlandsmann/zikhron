// #include <spdlog/spdlog.h>
// #include <utils/format.h>
//
// #include <algorithm>
// #include <boost/math/special_functions/beta.hpp>
// #include <concepts>
// #include <cstddef>
// #include <cstdlib>
// #include <functional>
// #include <iterator>
// #include <random>
// #include <ranges>
// #include <span>
// #include <vector>
//
// namespace ranges = std::ranges;
// namespace views = std::ranges::views;
//
// const static std::vector<double> probs =
//         {0.98, 0.96, 0.93, 0.90, 0.85, 0.80, 0.73, 0.60, 0.47, 0.35, 0.20};
//
// // {0.20, 0.35, 0.47, 0.60, 0.73, 0.80, 0.85, 0.90, 0.93, 0.96, 0.98};
//
// // {0.20, 0.25, 0.27, 0.20, 0.23, 0.20, 0.25, 0.20, 0.23, 0.26, 0.28};
//
// struct BetaState
// {
//     double alpha{1.0};
//     double beta{0.0};
//     double monteCarlo{0.0};
//     auto operator==(const BetaState& bt) const -> bool = default;
// };
//
// struct Pool
// {
//     int start, end;
//     double mean;
//     double total_weight; // alpha + beta sum
// };
//
// void printWeights(const std::vector<BetaState>& weights)
// {
//     for (const auto& w : weights) {
//         auto prob = w.alpha / (w.alpha + w.beta);
//         fmt::print("{:.3f}  ", prob);
//     }
//     fmt::print("\n");
// }
//
// // PAVA smoothing logic
// auto get_smoothed_states(const std::vector<BetaState>& raw_states) -> std::vector<BetaState>
// {
//     std::vector<Pool> pools;
//     for (int i = 0; i < raw_states.size(); ++i) {
//         pools.push_back({i, i, raw_states[i].alpha / (raw_states[i].alpha + raw_states[i].beta),
//                          raw_states[i].alpha + raw_states[i].beta});
//     }
//
//     bool merged = true;
//     while (merged) {
//         merged = false;
//         for (size_t i = 0; i < pools.size() - 1; ++i) {
//             if (pools[i].mean > pools[i + 1].mean) { // Monotonicity violation
//                 pools[i].end = pools[i + 1].end;
//                 double combined_weight = pools[i].total_weight + pools[i + 1].total_weight;
//                 double combined_alpha = (pools[i].mean * pools[i].total_weight) + (pools[i + 1].mean * pools[i + 1].total_weight);
//                 pools[i].mean = combined_alpha / combined_weight;
//                 pools[i].total_weight = combined_weight;
//                 pools.erase(pools.begin() + i + 1);
//                 merged = true;
//                 i = -1;
//             }
//         }
//     }
//
//     std::vector<BetaState> smoothed(raw_states.size());
//     for (const auto& p : pools) {
//         for (int i = p.start; i <= p.end; ++i) {
//             // Distribute pool weight back to states
//             double pool_size = (p.end - p.start + 1);
//             smoothed[i].alpha = (p.mean * p.total_weight) / pool_size;
//             smoothed[i].beta = ((1.0 - p.mean) * p.total_weight) / pool_size;
//         }
//     }
//     return smoothed;
// }
//
// auto calculateWeights(std::vector<BetaState>& weights) -> std::vector<BetaState>
// {
//     printWeights(weights);
//     auto stateOut = get_smoothed_states(weights);
//     printWeights(stateOut);
//     return stateOut;
// }
//
// struct BetaPropabiltiy
// {
//     double propability{};
//     double certainty{};
// };
//
// auto getPropabiltiy(const BetaState& weight) -> double
// {
//     return weight.alpha / (weight.alpha + weight.beta);
// }
//
// auto ibeta(const BetaState& weight, double propability) -> double
// {
//     return boost::math::ibeta(weight.alpha, weight.beta, propability);
// }
//
// auto getIbeta(const BetaState& weight) -> double
// {
//     double propability = getPropabiltiy(weight);
//     return ibeta(weight, propability);
// }
//
// auto getCertainty(const BetaState& weight, double propability) -> double
// {
//     double propabilityMin = std::max(0.0, propability - 0.02);
//     double propabilityMax = std::min(1.0, propability + 0.02);
//     return ibeta(weight, propabilityMax)
//            - ibeta(weight, propabilityMin);
// }
//
// auto getCertainty(const BetaState& weight) -> double
// {
//     double propability = getPropabiltiy(weight);
//     return getCertainty(weight, propability);
// }
//
// auto getBetaPropabiltiy(const BetaState& weight) -> BetaPropabiltiy
// {
//     double propability = getPropabiltiy(weight);
//     return {.propability = propability,
//             .certainty = getCertainty(weight, propability)};
// }
//
// auto avgIt(const std::vector<BetaState>& weights, const BetaState& w) -> std::vector<BetaState>::const_iterator
// {
//     auto itFirst = ranges::find(weights, w);
//     auto itLast = ranges::find_last(weights, w);
//
//     return ranges::next(itFirst, ranges::distance(itFirst, itLast.begin()) / 2);
// }
//
// template<std::ranges::input_range R>
//     requires std::same_as<std::ranges::range_value_t<R>, BetaState>
// auto getSampleIndex(R weights, std::mt19937& gen) -> std::size_t
// {
//     constexpr double target = 0.9;
//     std::size_t best_index = 0;
//     double min_dist = 2.0; // Max possible distance is 1.0
//
//     for (const auto& [index, weight] : views::enumerate(weights)) {
//         // Generate a random sample from Beta(alpha, beta)
//         std::gamma_distribution<double> dist_a(weight.alpha + 1, 1.0);
//         std::gamma_distribution<double> dist_b(weight.beta + 1, 1.0);
//
//         double a = dist_a(gen);
//         double b = dist_b(gen);
//         double beta_sample = a / (a + b);
//
//         double dist = std::abs(beta_sample - target);
//         if (dist < min_dist) {
//             min_dist = dist;
//             best_index = static_cast<std::size_t>(index);
//         }
//     }
//     return best_index;
// }
//
// auto monteCarlo(std::vector<BetaState>& weights, std::mt19937& gen) -> std::vector<double>
// {
//     std::vector<double> result(weights.size());
//     constexpr double runs = 1000;
//     for (int i = 0; i < runs; i++) {
//         auto index = static_cast<std::size_t>(getSampleIndex(weights, gen));
//         result.at(index)++;
//     }
//     for (const auto& [w, r] : views::zip(weights, result)) {
//         r = r / runs;
//         w.monteCarlo = r;
//     }
//     return result;
// }
//
// auto getBestIndex(std::vector<BetaState>& weights, std::mt19937& gen) -> std::size_t
// {
//     constexpr std::size_t startIndex = 0;
//     constexpr double target = 0.9;
//     auto mc = monteCarlo(weights, gen);
//     auto itPivot = ranges::find_if(weights, [](const BetaState& weight) -> bool { return getPropabiltiy(weight) < target; });
//     itPivot = std::max(itPivot, weights.begin() + 1);
//     if (itPivot == weights.end()) {
//         itPivot = ranges::min_element(weights, ranges::less{}, &getPropabiltiy);
//         itPivot = std::max(std::next(weights.begin(), startIndex), itPivot);
//     } else {
//         auto itPrevious = std::next(itPivot, -1);
//         itPrevious = std::max(weights.begin(), itPrevious);
//         if (itPrevious->monteCarlo > itPivot->monteCarlo) {
//             itPivot = itPrevious;
//         }
//     }
//     auto itBegin = std::max(weights.begin(), std::next(itPivot, -1));
//     auto itEnd = std::min(weights.end(), std::next(itBegin, 3));
//     auto distanceBegin = std::distance(weights.begin(), itBegin);
//     spdlog::info("Pivot: {}", std::distance(weights.begin(), itPivot));
//     // auto distancePivot = std::distance(weights.begin(), itPivot);
//     // auto itMc = ranges::max_element(ranges::subrange{mc.begin(), mc.begin() + distancePivot});
//     // auto startIndex = std::distance(mc.begin(), itMc) - 1;
//     // startIndex = std::clamp<long>(startIndex, 0, std::max<long>(0, static_cast<long>(weights.size() - 2)));
//     // auto endIndex = std::min(static_cast<long>(weights.size()), startIndex + 3);
//     //
//     // // auto startIndex =
//     // // spdlog::info("si: {}", startIndex);
//     return getSampleIndex(ranges::subrange{itBegin, itEnd}, gen)
//            + static_cast<std::size_t>(distanceBegin);
// }
//
// void run_simulation()
// {
//     constexpr int runs = 1000;
//     std::random_device rd;
//     std::mt19937 gen(rd());
//     std::uniform_int_distribution<std::size_t> distrib(0, probs.size() - 1);
//     auto weights = std::vector<BetaState>(probs.size());
//
//     // for (int run = 0; run < 10; run++) {
//     //     auto index = distrib(gen);
//     //     auto prop = probs.at(index);
//     //     auto& weight = weights.at(index);
//     //     std::bernoulli_distribution bernoulli{prop};
//     //     bool pass = bernoulli(gen);
//     //     if (pass) {
//     //         weight.alpha++;
//     //     } else {
//     //         weight.beta++;
//     //     }
//     // }
//
//     // auto tmpWeights = weights;
//     for (int run = 0; run < runs; run++) {
//         // auto index = distrib(gen);
//         // ranges::sort(tmpWeights, std::greater{}, getPropabiltiy);
//         // auto index = getBestIndex(tmpWeights, gen);
//         // ranges::sort(tmpWeights, std::greater{}, getPropabiltiy);
//         auto index = getBestIndex(weights, gen);
//         auto prop = probs.at(index);
//         auto& weight = weights.at(index);
//         // auto& tmpWeight = weights.at(index);
//         std::bernoulli_distribution bernoulli{prop};
//         bool pass = bernoulli(gen);
//         if (pass) {
//             weight.alpha++;
//             // tmpWeight.alpha++;
//         } else {
//             weight.beta++;
//             // tmpWeight.beta++;
//         }
//         spdlog::info("{:2d} tp: {:.3f} p: {:.3f} c:{:.3f}, ct:{:.3f}, count: {}",
//                      index, prop, getPropabiltiy(weight), getCertainty(weight), getCertainty(weight, 0.9), weight.alpha + weight.beta - 1);
//     }
//     calculateWeights(weights);
//     auto monte = monteCarlo(weights, gen);
//     std::vector<double> propabilities;
//     ranges::transform(weights, std::back_inserter(propabilities), getPropabiltiy);
//     spdlog::info("monte: {:.3f}", fmt::join(monte, ", "));
//     spdlog::info("prob:  {:.3f}", fmt::join(propabilities, ", "));
//     spdlog::info("truep: {:.3f}", fmt::join(probs, ", "));
//
//     // spdlog::info("weight | propability | diff");
//     // for (const auto& [w, p] : views::zip(weights, probs)) {
//     //     auto prob = w.alpha / (w.alpha + w.beta);
//     //     spdlog::info("{:.3f} | {:.3f}      | {:.3f} , {:.3f}  -  {}", prob, p, prob - p,
//     //                  boost::math::ibeta(w.alpha, w.beta, 0.92)
//     //                          - boost::math::ibeta(w.alpha, w.beta, 0.88),
//     //                  w.beta + w.alpha);
//     // }
//     // BetaState bt{.alpha = 1, .beta = 1};
//     // std::bernoulli_distribution bernoulli{0.95};
//     // for (int i = 0; i < 100; i++) {
//     //     if (bernoulli(gen)) {
//     //         bt.alpha++;
//     //     } else {
//     //         bt.beta++;
//     //     }
//     //     auto p = bt.alpha / (bt.alpha + bt.beta);
//     //     // spdlog::info("i: {}, {:.3f}", i, boost::math::ibeta(i, 1, 0.92) - boost::math::ibeta(i, 1, 0.88));
//     //     spdlog::info("i: {}, {:.3f} {:.3f}", i, bt.alpha / (bt.alpha + bt.beta),
//     //                  boost::math::ibeta(bt.alpha, bt.beta, p + 0.02) - boost::math::ibeta(bt.alpha, bt.beta, p - 0.02));
//     // }
// }
//
// auto main() -> int
// {
//     spdlog::info("hello");
//     run_simulation();
//     return 0;
//}
#include <algorithm>
#include <climits>
#include <iostream>
#include <numeric>
#include <vector>

struct Partition
{
    std::vector<std::vector<int>> indices;
    long long imbalance = LLONG_MAX;
    bool valid = false;
};

class Partitioner
{
    std::vector<int> data;
    int min_required;

public:
    Partitioner(std::vector<int> nums, int min_val)
        : data(nums), min_required(min_val) {}

    void findBestPartition(int start_idx, int k_remaining, std::vector<std::vector<int>>& current_partition, Partition& best)
    {
        // Base Case: No more sets to create
        if (k_remaining == 1) {
            std::vector<int> last_set;
            long long sum = 0;
            for (int i = start_idx; i < data.size(); ++i) {
                last_set.push_back(i);
                sum += data[i];
            }

            if (sum >= min_required) {
                current_partition.push_back(last_set);
                evaluate(current_partition, best);
                current_partition.pop_back();
            }
            return;
        }

        // Try every possible split point for the current set
        long long current_sum = 0;
        std::vector<int> current_indices;
        for (int i = start_idx; i <= (int)data.size() - k_remaining; ++i) {
            current_sum += data[i];
            current_indices.push_back(i);

            if (current_sum >= min_required) {
                current_partition.push_back(current_indices);
                findBestPartition(i + 1, k_remaining - 1, current_partition, best);
                current_partition.pop_back();
            }
        }
    }

private:
    void evaluate(const std::vector<std::vector<int>>& partition, Partition& best)
    {
        long long max_s = 0, min_s = LLONG_MAX;
        for (const auto& set : partition) {
            long long s = 0;
            for (int idx : set)
                s += data[idx];
            max_s = std::max(max_s, s);
            min_s = std::min(min_s, s);
        }

        long long diff = max_s - min_s;
        if (diff < best.imbalance) {
            best.imbalance = diff;
            best.indices = partition;
            best.valid = true;
        }
    }
};

int main()
{
    std::vector<int> numbers = {900, 23, 245, 400, 600, 26, 800, 300, 1100, 50, 950};
    int min_target = 1000;
    Partition absolute_best;

    Partitioner p(numbers, min_target);

    // Try splitting into 5 sets, then 4, 3, 2...
    for (int k = 5; k >= 2; --k) {
        std::vector<std::vector<int>> current;
        Partition best_for_k;
        p.findBestPartition(0, k, current, best_for_k);

        if (best_for_k.valid) {
            std::cout << "--- Best " << k << "-Set Partition ---" << std::endl;
            for (const auto& set : best_for_k.indices) {
                std::cout << "{ ";
                for (size_t i = 0; i < set.size(); ++i)
                    std::cout << set[i] << (i == set.size() - 1 ? "" : ", ");
                std::cout << " } ";
            }
            std::cout << "\nImbalance (Max-Min Sum): " << best_for_k.imbalance << "\n\n";
        }
    }

    return 0;
};
