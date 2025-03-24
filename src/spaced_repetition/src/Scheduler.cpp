#include "Scheduler.h"

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
#include <ratio>
#include <tuple>
#include <utility>

namespace ranges = std::ranges;
namespace views = std::views;

namespace sr {
Scheduler::Scheduler()
{
    initBins();
    // double ease = 1.;
    // ease = decreaseEase(ease, speedChange);
    // spdlog::info("EASE: {}", ease);
    // ease = decreaseEase(ease, speedChange);
    // spdlog::info("EASE: {}", ease);
    // ease = decreaseEase(ease, speedChange);
    // spdlog::info("EASE: {}", ease);
    // ease = decreaseEase(ease, speedChange);
    // spdlog::info("EASE: {}", ease);
    //
    // spdlog::info("-------------------");
    //
    // ease = increaseEaseTowardsNeutral(ease, speedChange).first;
    // spdlog::info("EASE: {}", ease);
    // ease = increaseEaseTowardsNeutral(ease, speedChange).first;
    // spdlog::info("EASE: {}", ease);
    // ease = increaseEaseTowardsNeutral(ease, speedChange).first;
    // spdlog::info("EASE: {}", ease);
    // ease = increaseEaseTowardsNeutral(ease, speedChange).first;
    // spdlog::info("EASE: {}", ease);
    // ease = increaseEaseTowardsNeutral(ease, speedChange).first;
    // spdlog::info("EASE: {}", ease);
    //
    // spdlog::info("-------------------");
    //
    // ease = increaseEase(ease, speedChange);
    // spdlog::info("EASE: {}", ease);
    // ease = increaseEase(ease, speedChange);
    // spdlog::info("EASE: {}", ease);
    // ease = increaseEase(ease, speedChange);
    // spdlog::info("EASE: {}", ease);
    // ease = increaseEase(ease, speedChange);
    // spdlog::info("EASE: {}", ease);
    // ease = decreaseEaseTowardsNeutral(ease, speedChange).first;
    // spdlog::info("EASE: {}", ease);
    // ease = decreaseEaseTowardsNeutral(ease, speedChange).first;
    // spdlog::info("EASE: {}", ease);
    // ease = decreaseEaseTowardsNeutral(ease, speedChange).first;
    // spdlog::info("EASE: {}", ease);
    // ease = decreaseEaseTowardsNeutral(ease, speedChange).first;
    // spdlog::info("EASE: {}", ease);
    //
    // spdlog::info("-------------------");
    // ease = decreaseEase(ease, speedStabilize);
    // spdlog::info("EASE: {}", ease);
    // ease = increaseEase(ease, speedChange);
    // spdlog::info("EASE: {}", ease);
    // ease = decreaseEase(ease, speedChange);
    // spdlog::info("EASE: {}", ease);
    //
    // stabilityFromInterval(365, 0.84);
}

auto Scheduler::review(const SpacedRepetitionData& srd,
                       Rating rating) const -> SpacedRepetitionData
{
    // SpacedRepetitionData result = srd;
    // using Days = std::chrono::duration<double, std::ratio<86400>>;
    // Days duration{srd.due - srd.reviewed};
    // result.stability = stabilityFromInterval(duration.count(), srd.ease);
    // spdlog::info("result stability: {}", result.stability);

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
    std::unreachable();
}

auto Scheduler::reviewStateNewWord(const SpacedRepetitionData& srd,
                                   Rating rating) const -> SpacedRepetitionData
{
    auto timeNow = std::chrono::system_clock::now();
    double stability = initialStability(rating);
    double ease = initialEase(rating);
    StudyState studyState{};

    std::chrono::duration<double> duration{};
    switch (rating) {
    case Rating::fail:
        duration = learnAgainTime;
        studyState = StudyState::learning;
        break;
    case Rating::pass:
        duration = learnSecondStepTime;
        studyState = StudyState::learning;
        break;
    case Rating::familiar:
        duration = getIntervalDays(stability, ease);
        studyState = StudyState::review;
        break;
    }
    return {
            .reviewed = timeNow,
            .due = normalizeDue(timeNow + std::chrono::duration_cast<std::chrono::nanoseconds>(duration)),
            .shiftBackward = rating == Rating::familiar ? 2 : 0,
            .shiftForward = rating == Rating::familiar ? 4 : 0,
            .ease = ease,
            .stability = stability,
            .state = studyState,
            .enabled = srd.enabled,
            .triggerCardIndices = srd.triggerCardIndices,
    };
}

auto Scheduler::reviewStateLearning(const SpacedRepetitionData& srd,
                                    Rating rating) const -> SpacedRepetitionData
{
    auto lastDuration = std::chrono::duration_cast<std::chrono::minutes>(srd.due - srd.reviewed);
    auto timeNow = std::chrono::system_clock::now();
    double stability = srd.stability;
    double ease = srd.ease;
    StudyState studyState{StudyState::learning};

    std::chrono::duration<double> duration{};
    switch (rating) {
    case Rating::fail:
        duration = learnAgainTime;
        break;
    case Rating::pass:
        duration = learnFirstStepTime;
        if (lastDuration == learnFirstStepTime) {
            duration = learnSecondStepTime;
        }
        if (lastDuration == learnSecondStepTime) {
            duration = getIntervalDays(srd.stability, ease);
            studyState = StudyState::review;
        }
        break;
    default:
        break;
    }
    return {
            .reviewed = timeNow,
            .due = normalizeDue(timeNow + std::chrono::duration_cast<std::chrono::nanoseconds>(duration)),
            .shiftBackward = srd.shiftBackward,
            .shiftForward = srd.shiftForward,
            .ease = ease,
            .stability = stability,
            .state = studyState,
            .enabled = srd.enabled,
            .triggerCardIndices = srd.triggerCardIndices,
    };
}

auto Scheduler::reviewStateReview(const SpacedRepetitionData& srd,
                                  Rating rating) const -> SpacedRepetitionData
{
    using Days = std::chrono::duration<double, std::ratio<86400>>;
    // result.reviewed = std::chrono::system_clock::now();

    auto timeNow = std::chrono::system_clock::now();
    double ease = 0;
    double stability = 0;
    // spdlog::info("old d: {}, d: {}, s: {}", srd.ease, ease, stability);
    StudyState studyState{StudyState::review};
    std::chrono::duration<double> duration{};
    switch (rating) {
    case Rating::fail:
        std::tie(stability, ease) = failStabilityAndEase(srd);
        duration = relearnAgainTime;
        studyState = StudyState::relearning;
        break;
    case Rating::pass: {
        Days reviewedDuration{std::chrono::system_clock::now() - srd.reviewed};
        auto reviewStability = stabilityFromInterval(reviewedDuration.count(), srd.ease);
        // in case the stability may not rise because interval days is unchanging, this should prevent unchanging stability
        // if (getIntervalDays(reviewStability, srd.ease) == getIntervalDays(srd)) {
        //     reviewStability = std::max(reviewStability, srd.stability);
        // }

        // spdlog::info("srd: {}, ", srd.serialize());
        // spdlog::info("result stability: {}, {}", reviewStability, reviewedDuration.count());
        ease = nextEase(reviewStability, srd.ease);
        auto tmpStability = nextStability(reviewStability, ease, rating);
        auto tmpInterval = getInterval(tmpStability, srd.ease);
        stability = stabilityFromInterval(tmpInterval, ease);

        // if the stability doesn't change, force a change. (if the ease doesn't change, stability should rise)
        // if (ease == srd.ease && stability - srd.stability < 0.01) {
        //     stability = nextStability(srd.stability, ease, rating);
        //     spdlog::info("force a change, stability: {}", stability);
        // }
        duration = getIntervalDays(stability, ease);
        break;
    }
    default:
        break;
    }
    return {
            .reviewed = timeNow,
            .due = normalizeDue(timeNow + std::chrono::duration_cast<std::chrono::nanoseconds>(duration)),
            .shiftBackward = shiftBackward(srd, stability, ease, rating),
            .shiftForward = shiftForward(srd, stability, ease, rating),
            .ease = ease,
            .stability = stability,
            .state = studyState,
            .enabled = srd.enabled,
            .triggerCardIndices = srd.triggerCardIndices,
    };
}

auto Scheduler::reviewStateRelearning(const SpacedRepetitionData& srd,
                                      Rating rating) const -> SpacedRepetitionData
{
    auto timeNow = std::chrono::system_clock::now();
    double stability = srd.stability;
    double ease = srd.ease;
    StudyState studyState{StudyState::relearning};

    std::chrono::duration<double> duration{};
    switch (rating) {
    case Rating::fail:
        duration = relearnAgainTime;
        break;
    case Rating::pass:
        stability = srd.stability;
        ease = srd.ease;
        duration = getIntervalDays(stability, ease);
        // spdlog::info("dur: {}", getInterval(stability, ease));
        studyState = StudyState::review;
        break;
    default:
        break;
    }
    return {
            .reviewed = timeNow,
            .due = normalizeDue(timeNow + std::chrono::duration_cast<std::chrono::nanoseconds>(duration)),
            .shiftBackward = srd.shiftBackward,
            .shiftForward = srd.shiftForward,
            .ease = ease,
            .stability = stability,
            .state = studyState,
            .enabled = srd.enabled,
            .triggerCardIndices = srd.triggerCardIndices,
    };
}

auto Scheduler::initialStability(Rating rating) const -> double
{
    switch (rating) {
    case Rating::familiar:
        return familiarStability;
    case Rating::pass:
        return 1.;
    default:
        return 0.;
    }
}

auto Scheduler::initialEase(Rating rating) const -> double
{
    using namespace std::numbers;

    switch (rating) {
    case Rating::familiar:
        return familiarEase;
    case Rating::pass:
        return 1.3;
    default:
        return 1.;
    }
}

auto Scheduler::nextStability(double stability,
                              double ease,
                              Rating rating) const -> double
{
    if (rating == Rating::fail) {
        if (stability >= 1.5) {
            stability = (stability + log(stability)) / 2.;
        } else {
            stability = 0;
        }
        return stability;
    }

    double resultStability = stability + getStep(ease);

    // for hard cards it may be, that it jumps from 1 day to 3 days. It should only go to 2 days max (double)
    if (ease < getStep(ease) && ease < 1.) {
        double oldInterval = getInterval(stability, ease);
        double newInterval = getInterval(resultStability, ease);
        if (oldInterval < newInterval / 2.) {
            return stabilityFromInterval(oldInterval * 2., ease);
        }
    }

    return resultStability;
}

auto Scheduler::getStep(double ease) const -> double
{
    double step{};

    if (ease <= 1.) {
        step += 0.5 + (ease / 2.);
        // step = std::max(0.75, step);
    } else {
        step += 1 + ((ease - 1) / 2.);
        // step = std::min(1.25, step);
    }
    return std::clamp(step, 0.75, 1.25);
}

auto Scheduler::failStabilityAndEase(const SpacedRepetitionData& srd) const -> std::pair<double, double>
{
    if (srd.stability < 1.5) {
        // spdlog::info("e: {}, newe: {}, s: {}", srd.ease, decreaseEase(srd.ease, speedChange), srd.stability);
        return {std::max(0., srd.stability - srd.ease),
                decreaseEase(srd.ease, speedChange)};
    }
    double interval = (srd.stability + log(srd.stability));
    double stabilityCounter = srd.stability;
    double ease = srd.ease;
    while (stabilityCounter > interval / 2) {
        stabilityCounter -= speedChange / speedStabilize;
        ease = increaseEase(ease, speedChange);
    }

    return {stabilityFromInterval(interval, ease), ease};
}

auto Scheduler::stabilityFromInterval(double interval, double ease) const -> double
{
    constexpr double skewForFloor = 0.1;
    double stability = 1;
    double oldStability = 0;
    double saveCount = 0;

    while (saveCount < 16 && std::abs(stability - oldStability) > 0.0001) {
        saveCount++;
        oldStability = stability;

        auto bin = getBin(stability, ease);
        stability = log(interval + skewForFloor) / log(bin);

        // spdlog::info("i: {:.3F}, ni: {:.3F}, s: {}", interval, getInterval(stability, ease), stability);
    }

    return std::clamp<double>(stability, 0, numberOfBins);
}

auto Scheduler::getBin(double stability, double ease) const -> double
{
    auto binIndexLow = static_cast<std::size_t>(std::floor(stability));
    auto binIndexHigh = static_cast<std::size_t>(std::ceil(stability));
    binIndexLow = std::clamp<std::size_t>(binIndexLow, 0, numberOfBins - 1);
    binIndexHigh = std::clamp<std::size_t>(binIndexHigh, 0, numberOfBins - 1);
    double binEase0 = ease > 1. ? binsEasy.at(binIndexLow) : binsHard.at(binIndexLow);
    double binEase1 = ease > 1. ? binsEasy.at(binIndexHigh) : binsHard.at(binIndexHigh);
    double binGood0 = binsGood.at(binIndexLow);
    double binGood1 = binsGood.at(binIndexHigh);

    double factEase = ease > 1. ? ease - 1 : 1 - ease;
    double factStability = stability - std::floor(stability);
    double bin0 = (binGood0 * (1. - factEase)) + (binEase0 * factEase);
    double bin1 = (binGood1 * (1. - factEase)) + (binEase1 * factEase);
    return (bin0 * (1. - factStability)) + (bin1 * factStability);
}

auto Scheduler::getInterval(double stability, double ease) const -> double
{
    double bin = getBin(stability, ease);
    double interval = std::pow(bin, stability);
    // spdlog::info("i: {}, bin: {}, stability: {}", interval, bin, stability);
    interval = std::clamp(interval, 1., maxInterval);
    return interval;
}

auto Scheduler::getIntervalDays(double stability, double ease) const -> std::chrono::days
{
    double interval = getInterval(stability, ease);
    return std::chrono::days{static_cast<int>(interval)};
}

auto Scheduler::normalizeDue(const SpacedRepetitionData::time_point& due) -> SpacedRepetitionData::time_point
{
    auto duration = due - std::chrono::system_clock::now();
    using Days = std::chrono::duration<double, std::ratio<86400>>;
    if (std::chrono::duration_cast<Days>(duration).count() > 0.9) {
        // return std::chrono::time_point_cast<std::chrono::days>(due);
    }
    return due;
}

auto Scheduler::getIntervalDays(const SpacedRepetitionData& srd) const -> std::chrono::days
{
    return getIntervalDays(srd.stability, srd.ease);
}

auto Scheduler::getRatingSuggestion(const SpacedRepetitionData& srd) const -> Rating
{
    switch (srd.state) {
    case StudyState::newWord:
        return Rating::fail;
    case StudyState::learning: {
        auto lastDuration = std::chrono::duration_cast<std::chrono::minutes>(srd.due - srd.reviewed);
        if (lastDuration < learnFirstStepTime) {
            return Rating::fail;
        }
        return Rating::pass;
    }
    case StudyState::relearning:
        return Rating::fail;
    case StudyState::review:
        return Rating::pass;
    }
    return Rating::pass;
}

auto Scheduler::increaseEase(double ease, double speed) -> double
{
    if (ease < 1.0) {
        std::tie(ease, speed) = increaseEaseTowardsNeutral(ease, speed);
    }
    if (speed == 0.) {
        return ease;
    }
    return std::min(2., ease + ((2. - ease) * speed));
}

auto Scheduler::decreaseEase(double ease, double speed) -> double
{
    if (ease > 1.0) {
        std::tie(ease, speed) = decreaseEaseTowardsNeutral(ease, speed);
    }
    // spdlog::info("decrease: e: {}, s: {}", ease, speed);
    if (speed == 0.) {
        return ease;
    }
    return std::max(0., ease - (ease * speed));
}

auto Scheduler::increaseEaseTowardsNeutral(double ease, double speed) -> std::pair<double, double>
{
    if (1. - ease < 0.01) {
        return {1., speed};
    }

    double oldEase = ease;
    ease = ease / (1 - speed);
    if (ease < 1.) {
        return {std::clamp(ease, 0.2, 1.), 0.};
    }

    if (ease - 1. < 0.01) {
        return {1., 0.};
    }
    speed = speed * ((speed - (1 - oldEase)) / speed);
    return {1., speed};
    // 0.2 * ((0.2 - (1 - 0.89)) / 0.2)
}

auto Scheduler::decreaseEaseTowardsNeutral(double ease, double speed) -> std::pair<double, double>
{
    if (ease - 1. < 0.01) {
        return {1., speed};
    }
    double oldEase = ease;
    ease = (ease - (2 * speed)) / (1 - speed);
    if (ease > 1.) {
        return {ease, 0.};
    }

    if (1. - ease < 0.01) {
        return {1., 0.};
    }
    speed = (speed * ((speed - oldEase) / speed)) + 1;
    return {1., speed};
    // 0.2 * ((0.2 - (1.09 )) / 0.2) + 1
}

auto Scheduler::nextEase(double stability, double ease) const -> double
{
    constexpr double stabilizeAt = 3.;
    if (std::max(stability, stability / getStep(ease)) < stabilizeAt) {
        return ease;
    }
    if (ease > 1.) {
        ease = decreaseEaseTowardsNeutral(ease, speedStabilize).first;
    }
    if (ease < 1.) {
        ease = increaseEaseTowardsNeutral(ease, speedStabilize).first;
    }
    return ease;

    // constexpr double factorFast = 1. / 5.;
    // constexpr double factorSlow = 0.5 / 5.;
    // double factor = (stability >= 4) && rating == Rating::pass ? factorSlow : factorFast;
    // if (rating == Rating::pass && std::max(stability, stability / getStep(ease)) >= 4) {
    //     if (std::abs(1. - ease) < factor) {
    //         ease = 1.;
    //     } else {
    //         rating = (ease > 1.) ? Rating::hard : Rating::easy;
    //     }
    // }
    // auto approach = [](double val, double factor) -> double {
    //     return val + ((1 - val) * factor);
    // };
    // auto revert = [](double val, double factor) -> double {
    //     return (val - factor) / (1 - factor);
    // };
    // const auto& fun = ((ease >= 1) == (rating == Rating::easy))
    //                           ? approach
    //                           : revert;
    // switch (rating) {
    // case Rating::fail:
    // case Rating::pass:
    //     return ease;
    // case Rating::hard:
    // case Rating::easy:
    //     if (ease >= 1) {
    //         double val = ease - 1.;
    //         val = fun(val, factor) + 1;
    //         if (rating == Rating::hard) {
    //             return std::clamp(val, 1 - factor, 2.);
    //         }
    //         return std::clamp(val, 1., 2.);
    //     } else {
    //         double val = 1 - ease;
    //         val = 1 - fun(val, factor);
    //         if (rating == Rating::hard) {
    //             return std::clamp(val, 0., 1.);
    //         }
    //         return std::clamp(val, 0., 1. + factor);
    //     }
    // }
    // std::unreachable();
}

auto Scheduler::shiftBackward(const SpacedRepetitionData& srd,
                              double stability,
                              double ease,
                              Rating rating) const -> int
{
    switch (rating) {
    case Rating::fail: {
        const auto& [failEase, failStability] = failStabilityAndEase(srd);
        double newInterval = getInterval(failStability, failEase);
        return static_cast<int>(newInterval / 4.);
    }

    case Rating::pass: {
        double easeHard = decreaseEase(srd.ease, speedChange);
        easeHard = decreaseEase(easeHard, speedChange);
        double stabilityHard = nextStability(srd.stability, easeHard, rating);
        double intervalHard = getInterval(stabilityHard, easeHard);
        if (intervalHard == maxInterval) {
            intervalHard *= 0.75;
        }
        double interval = getInterval(stability, ease);
        // spdlog::info("ih{:.2F}:ig{:.2F}, dh{:.2F}, sh{:.2F}", intervalHard, interval, easeHard, stabilityHard);
        // return static_cast<int>(std::max((interval - intervalHard), 0.));
        return static_cast<int>(std::clamp((interval - intervalHard), 0., interval / 4));
    }
    default:
        return 0;
    }
    std::unreachable();
}

auto Scheduler::shiftForward(const SpacedRepetitionData& srd,
                             double stability,
                             double ease,
                             Rating rating) const -> int
{
    switch (rating) {
    case Rating::fail: {
        const auto& [failEase, failStability] = failStabilityAndEase(srd);
        double newInterval = getInterval(failStability, failEase);
        return static_cast<int>(newInterval / 4.);
    }

    case Rating::pass: {
        double easeEasy = increaseEase(srd.ease, speedChange);
        // easeEasy = increaseEase(easeEasy, speedChange);
        double stabilityEasy = nextStability(srd.stability, easeEasy, rating);
        double intervalEasy = getInterval(stabilityEasy, easeEasy);
        double interval = getInterval(stability, ease);
        if (interval == maxInterval) {
            intervalEasy *= 1.25;
        }
        // spdlog::info("intervalEasy{:.2F}, interval{:.2F}, easeEasy{:.2F}, stabilityEasy{:.2F}", intervalEasy, interval, easeEasy, stabilityEasy);
        return static_cast<int>(std::clamp((intervalEasy - interval), 0., interval / 3));
        // double easeEasy = increaseEase(srd.ease, speedChange);
        //  easeEasy = increaseEase(easeEasy, speedChange);
        // double stabilityEasy = nextStability(srd.stability, easeEasy, Rating::pass);
        // double difficultyEasy = increaseEase(srd.ease, speedChange);
        // double stability = nextStability(srd.stability, difficultyEasy, Rating::pass);
        // double interval = nextInterval(stabilityGood, easeGood);
        // double intervalEasy = nextInterval(stabilityEasy, difficultyEasy);
        // return static_cast<int>(std::max((intervalEasy - intervalGood) / 3., 0.));
    }
    default:
        return 0;
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
        auto expNext = static_cast<double>(index + 1);
        auto exp = static_cast<double>(index);
        auto bin = binsGood.at(index);
        index++;

        return std::pow(std::pow(bin, expNext), 1. / exp);
    });
    binsEasy.back() = binsEasy.at(numberOfBins - 2);
    // double exp = 0;
    // for (const auto& [hard, good, easy] : views::zip(binsHard, binsGood, binsEasy)) {
    //     spdlog::info("{} {} {} - {}", hard, good, easy, std::pow(easy, exp));
    //     exp++;
    // }
}
} // namespace sr
