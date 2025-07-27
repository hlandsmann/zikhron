#include "user.h"

#include <cmath>
#include <random>

auto User::review(const SRS_data& srs_data, double day) -> bool
{
  return false;
}

auto User::exponentialDecay(double high, double low, double delta, double val) -> double
{
    if (delta <= 0 || high <= low || low <= 0) {
        return 0.0; // Invalid input handling
    }
    double k = std::log(high / low) / delta;
    return high * std::exp(-k * val);
}

auto User::genUserReviewData() -> UserReviewData
{
    std::uniform_real_distribution<> dist(0.0, 1.0);
    return {.difficulty = dist(mt19937), .memory = 0};
}
