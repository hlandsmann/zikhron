#include "Scheduler.h"

#include <annotation/Ease.h>
#include <database/SpacedRepetitionData.h>
#include <utils/Time.h>
#include <utils/format.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <iterator>
#include <numbers>
#include <ranges>
#include <tuple>
#include <utility>

namespace ranges = std::ranges;
namespace views = std::views;

namespace sr {
Scheduler::Scheduler()
{
    initBins();
}

auto Scheduler::review(const SpacedRepetitionData& srd,
                       Rating rating) const -> SpacedRepetitionData
{
    SpacedRepetitionData result{};

    using database::StudyState;
    switch (srd.state) {
    case StudyState::newWord:
        return reviewStateNewWord(srd, rating);
    case StudyState::learning:
        return reviewStateLearning(srd, rating);
    case StudyState::review:
        return reviewStateReview(srd, rating);
    case StudyState::relearning:
        return reviewStateRelearning(srd, rating);
    }
    return result;
}

auto Scheduler::reviewStateNewWord(const SpacedRepetitionData& srd,
                                   Rating rating) const -> SpacedRepetitionData
{
    auto timeNow = std::chrono::system_clock::now();
    double stability = initialStability(rating);
    double difficulty = initialDifficulty(rating);
    StudyState studyState{};

    std::chrono::duration<double> duration{};
    switch (rating) {
    case Rating::again:
        duration = learnAgainTime;
        studyState = StudyState::learning;
        break;
    case Rating::hard:
        duration = learnHardTime;
        studyState = StudyState::learning;
        break;
    case Rating::good:
        duration = learnGoodTime;
        studyState = StudyState::learning;
        break;
    case Rating::easy:
        duration = nextIntervalDays(stability, difficulty);
        studyState = StudyState::review;
        break;
    }
    return {
            .reviewed = timeNow,
            .due = timeNow + std::chrono::duration_cast<std::chrono::nanoseconds>(duration),
            .shiftBackward = rating == Rating::easy ? 2 : 0,
            .shiftForward = rating == Rating::easy ? 4 : 0,
            .difficulty = difficulty,
            .stability = stability,
            .state = studyState,
            .triggerCardIndices = srd.triggerCardIndices,
            .enabled = {},
    };
}

auto Scheduler::reviewStateLearning(const SpacedRepetitionData& srd,
                                    Rating rating) const -> SpacedRepetitionData
{
    auto lastDuration = std::chrono::duration_cast<std::chrono::minutes>(srd.due - srd.reviewed);
    auto timeNow = std::chrono::system_clock::now();
    double stability = srd.stability;
    double difficulty = srd.difficulty;
    StudyState studyState{StudyState::learning};

    std::chrono::duration<double> duration{};
    switch (rating) {
    case Rating::again:
        duration = learnAgainTime;
        break;
    case Rating::hard:
        duration = learnHardTime;
        if (lastDuration == learnGoodTime) {
            duration = learnGoodTime;
        }
        break;
    case Rating::good:
        duration = learnGoodTime;
        if (lastDuration == learnGoodTime) {
            duration = nextIntervalDays(srd.stability, difficulty);
            studyState = StudyState::review;
        }
        break;
    case Rating::easy:
        stability = nextStability(stability, difficulty, Rating::easy);
        duration = nextIntervalDays(stability, difficulty);
        studyState = StudyState::review;
        break;
    }
    return {
            .reviewed = timeNow,
            .due = timeNow + std::chrono::duration_cast<std::chrono::nanoseconds>(duration),
            .shiftBackward = srd.shiftBackward,
            .shiftForward = srd.shiftForward,
            .difficulty = difficulty,
            .stability = stability,
            .state = studyState,
            .triggerCardIndices = srd.triggerCardIndices,
            .enabled = {},
    };
}

auto Scheduler::reviewStateReview(const SpacedRepetitionData& srd,
                                  Rating rating) const -> SpacedRepetitionData
{
    auto lastDuration = std::chrono::duration_cast<std::chrono::minutes>(srd.due - srd.reviewed);
    auto timeNow = std::chrono::system_clock::now();
    double difficulty = nextDifficulty(srd.stability, srd.difficulty, rating);
    double stability = 0;
    // spdlog::info("old d: {}, d: {}, s: {}", srd.difficulty, difficulty, stability);
    StudyState studyState{StudyState::review};
    std::chrono::duration<double> duration{};
    switch (rating) {
    case Rating::again:
        // stability = nextStability(srd.stability, difficulty, rating);
        std::tie(stability, difficulty) = failStabilityAndDifficulty(srd);
        duration = relearnAgainTime;
        studyState = StudyState::relearning;
        break;
    case Rating::hard: {
        double temporalDifficulty = nextDifficulty(stability, difficulty, Rating::hard);
        stability = nextStability(srd.stability, temporalDifficulty, rating);
        duration = nextIntervalDays(stability, difficulty);
    } break;
    case Rating::good:
        stability = nextStability(srd.stability, difficulty, rating);
        duration = nextIntervalDays(stability, difficulty);
        break;
    case Rating::easy: {
        double temporalDifficulty = nextDifficulty(stability, difficulty, Rating::easy);
        stability = nextStability(srd.stability, temporalDifficulty, rating);
        duration = nextIntervalDays(stability, difficulty);
    } break;
    }
    return {
            .reviewed = timeNow,
            .due = timeNow + std::chrono::duration_cast<std::chrono::nanoseconds>(duration),
            .shiftBackward = shiftBackward(srd, stability, difficulty, rating),
            .shiftForward = {},
            .difficulty = difficulty,
            .stability = stability,
            .state = studyState,
            .triggerCardIndices = srd.triggerCardIndices,
            .enabled = {},
    };
}

auto Scheduler::reviewStateRelearning(const SpacedRepetitionData& srd,
                                      Rating rating) const -> SpacedRepetitionData
{
    auto lastDuration = std::chrono::duration_cast<std::chrono::minutes>(srd.due - srd.reviewed);
    auto timeNow = std::chrono::system_clock::now();
    double stability = srd.stability;
    double difficulty = srd.difficulty;
    StudyState studyState{StudyState::learning};

    std::chrono::duration<double> duration{};
    switch (rating) {
    case Rating::again:
        duration = relearnAgainTime;
        break;
    case Rating::hard:
        duration = relearnHardTime;
        break;
    case Rating::good:
        duration = nextIntervalDays(stability, difficulty);
        studyState = StudyState::review;
        break;
    case Rating::easy:
        stability = nextStability(stability, difficulty, Rating::easy);
        duration = nextIntervalDays(stability, difficulty);
        studyState = StudyState::review;
        break;
    }
    return {
            .reviewed = timeNow,
            .due = timeNow + std::chrono::duration_cast<std::chrono::nanoseconds>(duration),
            .shiftBackward = srd.shiftBackward,
            .shiftForward = srd.shiftForward,
            .difficulty = difficulty,
            .stability = stability,
            .state = studyState,
            .triggerCardIndices = srd.triggerCardIndices,
            .enabled = {},
    };
}

auto Scheduler::initialStability(Rating rating) const -> double
{
    switch (rating) {
    case Rating::easy:
        return 2.;
    case Rating::good:
        return 1.;
    default:
        return 0.;
    }
}

auto Scheduler::nextStability(double stability,
                              double difficulty,
                              Rating rating) const -> double
{
    double oldStability = stability;
    if (rating == Rating::again) {
        // stability = stability / 6.; // std::pow(stability,0.3);
        // stability = std::pow(stability, 0.3);
        // stability = std::log(stability);
        if (stability >= 1.5) {
            stability = (stability + log(stability)) / 2.;
        } else {
            stability = 0;
        }
        // spdlog::info("stability: {} -> {} ---------------------------------", oldStability, stability);
        return stability;
    }
    constexpr auto stabilityPenalty = 0.3;
    constexpr auto stabilityReward = 0.3;

    if (rating == Rating::hard) {
        stability -= stabilityPenalty;
    }
    if (rating == Rating::easy) {
        stability += stabilityReward;
    }
    return stability + difficulty;
}

auto Scheduler::failStabilityAndDifficulty(const SpacedRepetitionData& srd) const -> std::pair<double, double>
{
    if (srd.stability < 1.5) {
        return {std::max(0., srd.stability - srd.difficulty),
                nextDifficulty(srd.stability, srd.difficulty, Rating::hard)};
    }
    double stability = (srd.stability + log(srd.stability)) / 2.;
    double stabilityCounter = srd.stability;
    double difficulty = srd.difficulty;
    while (stabilityCounter > stability) {
        stabilityCounter -= 1.;
        difficulty = nextDifficulty(stability, difficulty, Rating::easy);
    }

    return {std::max(0., stability - (difficulty - srd.difficulty)), difficulty};
}

auto Scheduler::nextInterval(double stability, double difficulty) const -> double
{
    auto binIndexLow = static_cast<std::size_t>(std::floor(stability));
    auto binIndexHigh = static_cast<std::size_t>(std::ceil(stability));
    binIndexLow = std::clamp<std::size_t>(binIndexLow, 0, numberOfBins - 1);
    binIndexHigh = std::clamp<std::size_t>(binIndexHigh, 0, numberOfBins - 1);
    double binDifficulty0 = difficulty > 1. ? binsEasy.at(binIndexLow) : binsHard.at(binIndexLow);
    double binDifficulty1 = difficulty > 1. ? binsEasy.at(binIndexHigh) : binsHard.at(binIndexHigh);
    double binGood0 = binsGood.at(binIndexLow);
    double binGood1 = binsGood.at(binIndexHigh);

    double factDifficulty = difficulty > 1. ? difficulty - 1 : 1 - difficulty;
    double factStability = stability - std::floor(stability);
    double bin0 = (binGood0 * (1. - factDifficulty)) + (binDifficulty0 * factDifficulty);
    double bin1 = (binGood1 * (1. - factDifficulty)) + (binDifficulty1 * factDifficulty);
    double bin = (bin0 * (1. - factStability)) + (bin1 * factStability);

    double interval = std::pow(bin, stability);
    // spdlog::info("i: {}, bin: {}, stability: {}", interval, bin, stability);
    interval = std::clamp(std::floor(interval), 1., maxInterval);
    return interval;
}

auto Scheduler::nextIntervalDays(double stability, double difficulty) const -> std::chrono::days
{
    double interval = nextInterval(stability, difficulty);
    return std::chrono::days{static_cast<int>(interval)};
}

auto Scheduler::nextDifficulty(double stability, double difficulty,
                               Rating rating) const -> double
{
    constexpr double factorFast = 1. / 5.;
    constexpr double factorSlow = 0.5 / 5.;
    double factor = (stability >= 4) && rating == Rating::good ? factorSlow : factorFast;
    if (rating == Rating::good && stability >= 4) {
        if (std::abs(1. - difficulty) < factor) {
            difficulty = 1.;
        } else {
            rating = (difficulty > 1.) ? Rating::hard : Rating::easy;
        }
    }
    auto approach = [](double val, double factor) -> double {
        return val + ((1 - val) * factor);
    };
    auto revert = [](double val, double factor) -> double {
        return (val - factor) / (1 - factor);
    };
    const auto& fun = ((difficulty >= 1) == (rating == Rating::easy))
                              ? approach
                              : revert;
    switch (rating) {
    case Rating::again:
    case Rating::good:
        return difficulty;
    case Rating::hard:
    case Rating::easy:
        if (difficulty >= 1) {
            double val = difficulty - 1.;
            val = fun(val, factor) + 1;
            if (rating == Rating::hard) {
                return std::clamp(val, 1 - factor, 2.);
            }
            return std::clamp(val, 1., 2.);
        } else {
            double val = 1 - difficulty;
            val = 1 - fun(val, factor);
            if (rating == Rating::hard) {
                return std::clamp(val, 0., 1.);
            }
            return std::clamp(val, 0., 1. + factor);
        }
    }
    // auto linearDamping = [](double deltaDifficulty, double difficulty) -> double {
    //     return (10. - difficulty) * deltaDifficulty / 9.;
    // };
    // auto meanReversion = [this](double val1, double val2) -> double {
    //     return (weights[7] * val1) + ((1 - weights[7]) * val2);
    // };
    // double val1 = initialDifficulty(Rating::easy);
    // double deltaDifficulty = -(weights[6] * (static_cast<double>(rating) - 2));
    // double val2 = difficulty + linearDamping(deltaDifficulty, difficulty);
    // difficulty = meanReversion(val1, val2);
    std::unreachable();
}

auto Scheduler::initialDifficulty(Rating rating) const -> double
{
    using namespace std::numbers;

    switch (rating) {
    case Rating::easy:
        return 2.0;
    case Rating::good:
        return 1.3;
    default:
        return 1.;
    }
}

auto Scheduler::shiftBackward(const SpacedRepetitionData& srd,
                              double stability,
                              double difficulty,
                              Rating rating) const -> int
{
    switch (rating) {
    case Rating::again:
    case Rating::hard: {
        double oldInterval = nextInterval(srd.stability, srd.difficulty);
        double newInterval = nextInterval(stability, difficulty);
        return static_cast<int>(std::max<double>(newInterval - oldInterval, 0));
    }

    case Rating::good: {
        double difficultyHard = nextDifficulty(srd.stability, srd.difficulty, Rating::hard);
        double stabilityHard = nextStability(srd.stability, difficultyHard, Rating::hard);
        double intervalHard = nextInterval(stabilityHard, difficultyHard);
        double intervalGood = nextInterval(stability, difficulty);
        spdlog::info("ih{}:ig{}, dh{}, sh{}", intervalHard, intervalGood, difficultyHard, stabilityHard);
        return static_cast<int>(std::max((intervalGood - intervalHard)/2., 0.));
    }
    case Rating::easy: {
        double difficultyGood = nextDifficulty(srd.stability, srd.difficulty, Rating::good);
        double stabilityGood = nextStability(srd.stability, difficultyGood, Rating::good);
        double intervalGood = nextInterval(stabilityGood, difficultyGood);
        double intervalEasy = nextInterval(stability, difficulty);
        return static_cast<int>(std::max((intervalEasy - intervalGood)*2./3., 0.));
    }
    }
    std::unreachable();
}

void Scheduler::initBins()
{
    // ranges::fill(binsHard, 1.3);
    ranges::fill(binsGood, 2.0);
    // ranges::fill(binsEasy, 4.0);
    std::generate(std::next(binsHard.begin()), binsHard.end(), [this, index = std::size_t{0}]() mutable -> double {
        auto expPrev = static_cast<double>(index);
        auto exp = static_cast<double>(index + 1);
        auto bin = binsGood.at(index);
        index++;

        return std::pow(std::pow(bin, expPrev), 1. / exp);
    });
    std::generate(binsEasy.begin(), std::prev(binsEasy.end()), [this, index = std::size_t{1}]() mutable -> double {
        auto expNext = static_cast<double>(index);
        auto exp = static_cast<double>(index - 1);
        auto bin = binsGood.at(index);
        index++;

        return std::pow(std::pow(bin, expNext), 1. / exp);
    });
    binsEasy.back() = binsEasy.at(numberOfBins - 2);
    double exp = 0;
    for (const auto& [hard, good, easy] : views::zip(binsHard, binsGood, binsEasy)) {
        // spdlog::info("{} {} {} - {}", hard, good, easy, std::pow(easy, exp));
        exp++;
    }
}
} // namespace sr
