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
#include <numbers>

namespace sr {
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

    auto interval = nextInterval(srd.stability).count();
    spdlog::info("interval: {}", interval);

    double retrievability = getRetrievability(srd);
    spdlog::info("retrievability: {:.1F}", retrievability);
    double difficulty = srd.difficulty;
    // double stability = nextForgetStability(difficulty, srd.stability, retrievability, rating);
    using namespace std::chrono_literals;
    double stability = nextStability(difficulty, srd.stability, retrievability, rating);
    interval = nextInterval(stability).count();
    spdlog::info("interval: {}", interval);
    difficulty = nextDifficulty(difficulty, rating);
    spdlog::info("difficulty: {:.1F}", difficulty);

    spdlog::info("time: {}", utl::serializeTimePoint(srd.due));
    spdlog::info("time: {}", utl::serializeTimePoint(srd.due + learnGoodTime));
    auto forw = srd.due + learnGoodTime;
    spdlog::info("minutes: {}", std::chrono::duration_cast<std::chrono::minutes>(forw - srd.due).count());

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
        duration = nextInterval(stability);
        studyState = StudyState::review;
        break;
    }
    return {
            .reviewed = timeNow,
            .due = timeNow + std::chrono::duration_cast<std::chrono::nanoseconds>(duration),
            .shiftBackward = {},
            .shiftForward = {},
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
            duration = nextInterval(srd.stability);
            studyState = StudyState::review;
        }
        break;
    case Rating::easy:
        stability = nextStability(difficulty, stability, desiredRetention, Rating::easy);
        duration = nextInterval(stability);
        studyState = StudyState::review;
        break;
    }
    return {
            .reviewed = timeNow,
            .due = timeNow + std::chrono::duration_cast<std::chrono::nanoseconds>(duration),
            .shiftBackward = {},
            .shiftForward = {},
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
    double retrievability = getRetrievability(srd);
    retrievability = 0.9;
    double stability = nextStability(srd.difficulty, srd.stability, retrievability, rating);
    double difficulty = nextDifficulty(srd.difficulty, rating);
    StudyState studyState{StudyState::review};

    std::chrono::duration<double> duration{};
    switch (rating) {
    case Rating::again:
        duration = relearnAgainTime;
        studyState = StudyState::relearning;
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
            duration = nextInterval(srd.stability);
            studyState = StudyState::review;
        }
        break;
    case Rating::easy:
        stability = nextStability(difficulty, stability, desiredRetention, Rating::easy);
        duration = nextInterval(stability);
        studyState = StudyState::review;
        break;
    }
    return {
            .reviewed = timeNow,
            .due = timeNow + std::chrono::duration_cast<std::chrono::nanoseconds>(duration),
            .shiftBackward = {},
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
    switch (rating) {
    case Rating::again:
    case Rating::hard:
    case Rating::good:
    case Rating::easy:
        break;
    }
    return {};
}

auto Scheduler::getRetrievability(const SpacedRepetitionData& srd) const -> double
{
    double elapsedDays = nextInterval(srd.stability).count();
    return std::pow((factor * (elapsedDays + 1) / srd.stability), decay);
}

auto Scheduler::initialStability(Rating rating) const -> double
{
    double stability = weights.at(static_cast<std::size_t>(rating));
    stability = std::max(stability, 0.1);
    return stability;
}

auto Scheduler::nextForgetStability(double difficulty,
                                    double stability,
                                    double retrievability,
                                    Rating rating) const -> double
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

auto Scheduler::nextRecallStability(double difficulty,
                                    double stability,
                                    double retrievability,
                                    Rating rating) const -> double
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

auto Scheduler::nextStability(double difficulty,
                              double stability,
                              double retrievability,
                              Rating rating) const -> double
{
    return nextRecallStability(difficulty, stability, retrievability, rating);
}

auto Scheduler::nextInterval(double stability) const -> std::chrono::days
{
    double interval = ((stability / factor) * std::pow(desiredRetention, 1 / decay)) - 1;
    interval = std::clamp(std::ceil(interval), 1., maxInterval);

    return std::chrono::days{static_cast<int>(interval)};
}

auto Scheduler::nextDifficulty(double difficulty,
                               Rating rating) const -> double
{
    auto linearDamping = [](double deltaDifficulty, double difficulty) -> double {
        return (10. - difficulty) * deltaDifficulty / 9.;
    };
    auto meanReversion = [this](double val1, double val2) -> double {
        return (weights[7] * val1) + ((1 - weights[7]) * val2);
    };
    double val1 = initialDifficulty(Rating::easy);
    double deltaDifficulty = -(weights[6] * (static_cast<double>(rating) - 2));
    double val2 = difficulty + linearDamping(deltaDifficulty, difficulty);
    difficulty = meanReversion(val1, val2);

    return std::clamp(difficulty, 1., 10.);
}

auto Scheduler::initialDifficulty(Rating rating) const -> double
{
    using namespace std::numbers;

    double difficulty = weights[4] - std::pow(e, weights[5] * static_cast<double>(rating)) + 1;
    return std::clamp(difficulty, 1., 10.);
}

} // namespace sr
