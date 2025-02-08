#pragma once
#include <array>
#include <cstddef>

namespace database {
using SrsWeights = std::array<double, 19>;
// constexpr SrsWeights defaultSrsWeights = {
//         0.40255,
//         1.18385,
//         3.173,
//         15.69105,
//         7.1949,
//         0.5345,
//         1.4604,
//         0.0046,
//         1.54575,
//         0.1192,
//         1.01925,
//         1.9395,
//         0.11,
//         0.29605,
//         2.2698,
//         0.2315,
//         2.9898,
//         0.51655,
//         0.6621,
// };
constexpr SrsWeights defaultSrsWeights = {
        0.0748,
        0.2150,
        0.5703,
        2.1219,
        5.2418,
        1.3098,
        0.9775,
        0.0000,
        1.5674,
        0.0567,
        0.9661,
        2.1868,
        0.0100,
        0.4045,
        1.6662,
        0.1472,
        2.9544,
};

struct Weight
{
    static constexpr std::size_t initialStability_ratingAgain = 0;
    static constexpr std::size_t initialStability_ratingHard = 1;
    static constexpr std::size_t initialStability_ratingGood = 2;
    static constexpr std::size_t initialStability_ratingEasy = 3;
    static constexpr std::size_t initialDifficulty_lowValue = 4;
    static constexpr std::size_t initialDifficulty_highValue = 5;
};
} // namespace database
