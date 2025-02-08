#include "Scheduler.h"

#include <annotation/Ease.h>
#include <utils/format.h>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace sr {
auto Scheduler::review(const SpacedRepetitionData& srd, Rating rating) const -> SpacedRepetitionData
{
    SpacedRepetitionData result{};

    double interval = nextInterval(srd.stability);
    spdlog::info("interval: {:.1F}", interval);

    double retrievability = getRetrievability(srd);
    spdlog::info("retrievability: {:.1F}", retrievability);
    double difficulty = srd.difficulty;
    double stability = nextForgetStability(difficulty, srd.stability, retrievability, rating);
    // double stability = nextStability(difficulty, srd.stability, retrievability, rating);
    interval = nextInterval(stability);
    spdlog::info("interval: {:.1F}", interval);

    return result;
}

auto Scheduler::getRetrievability(const SpacedRepetitionData& srd) const -> double
{
    double elapsedDays = nextInterval(srd.stability);
    return std::pow((factor * (elapsedDays + 1) / srd.stability), decay);
}

auto Scheduler::nextForgetStability(double difficulty, double stability, double retrievability, Rating rating) const -> double
{
    using namespace std::numbers;
    // clang-format off
    double shortTerm = weights[11]
        * (std::pow(difficulty, -weights[12]) )
        * (std::pow(stability + 1, weights[13]) - 1)
        *  std::pow(e, (1-retrievability) * weights[14]);
    double longTerm = stability /
          std::pow(e, (weights[17] * weights[18]));
    // clang-format on
    return std::min(shortTerm, longTerm);
}

auto Scheduler::nextRecallStability(double difficulty, double stability, double retrievability, Rating rating) const -> double
{
    using namespace std::numbers;
    double hardPenalty = rating == Rating::hard ? weights[15] : 1.;
    double easyBonus = rating == Rating::easy ? weights[16] : 1.;
    // clang-format off
    return stability * (
      1
      +  std::pow(e, weights[8])
      * (11 - difficulty)
      *  std::pow(stability, -weights[9])
      * (std::pow(e, (1 - retrievability) * weights[10]) - 1.)
      *  hardPenalty
      *  easyBonus
   );
    // clang-format on
}

auto Scheduler::nextStability(double difficulty, double stability, double retrievability, Rating rating) const -> double
{
    return nextRecallStability(difficulty, stability, retrievability, rating);
}

auto Scheduler::nextInterval(double stability) const -> double
{
    double interval = ((stability / factor) * std::pow(desiredRetention, 1 / decay)) - 1;
    interval = std::clamp(std::ceil(interval), 1., maxInterval);

    return interval;
}

} // namespace sr
