#include <Ease.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <span>
#include <utility>

namespace ranges = std::ranges;

Ease::Ease(float intervalDay, int dueDays, float easeFactor)
    : easeVal{Rating::easy}
    , progress{.intervalDay = intervalDay,
               .dueDays = dueDays,
               .easeFactor = easeFactor}
{
    if (easeFactor <= thresholdFactorGood) {
        easeVal = Rating::good;
    }
    if (easeFactor <= thresholdFactorHard && intervalDay < thresholdIntervalHard) {
        easeVal = Rating::hard;
    }
    if (easeFactor <= thresholdFactorAgain && intervalDay < thresholdIntervalAgain) {
        easeVal = Rating::again;
    }
}

auto computeProgress(Rating ease, Ease::Progress progress) -> Ease::Progress
{
    float easeChange = [=]() -> float {
        switch (ease) {
        case Rating::easy:
            return Ease::changeFactorEasy;
        case Rating::good:
            return Ease::changeFactorGood;
        case Rating::hard:
        case Rating::again:
            return Ease::changeFactorHard;
        }
        std::unreachable();
    }();
    float easeFactor = std::clamp(std::max(Ease::minEaseFactor, progress.easeFactor) * easeChange,
                                  Ease::minEaseFactor,
                                  Ease::maxEaseFactor);

    float tempEaseFactor = [=]() -> float {
        switch (ease) {
        case Rating::easy:
        case Rating::good:
            return easeFactor;
        case Rating::hard:
            return Ease::tempEaseFactorHard;
        default:
            return 0.F;
        }
    }();

    float intervalDay = 0.F;
    if (tempEaseFactor != 0) {
        float partIntervalDay = std::clamp(progress.intervalDay - static_cast<float>(progress.dueDays),
                                           0.F,
                                           progress.intervalDay);
        float partEaseFactor = tempEaseFactor - 1.F;
        intervalDay = progress.intervalDay + partEaseFactor * partIntervalDay;
    }
    return {.intervalDay = intervalDay,
            .dueDays = 0,
            .easeFactor = easeFactor};
}

auto Ease::getProgress() const -> Progress
{
    std::array easeValList = {Rating::again, Rating::hard, Rating::good, Rating::easy};
    std::array<float, easeValList.size()> intervals{};
    ranges::transform(easeValList, intervals.begin(), [this](Rating ease) {
        return computeProgress(ease, progress).intervalDay;
    });

    auto intervalSpan = std::span(std::next(intervals.begin()), intervals.end());
    for (auto it = intervalSpan.begin(); it < intervalSpan.end(); it++) {
        *it = std::max(*it, *std::prev(it) + 1.F);
    }

    float intervalDay = intervals.at(mapEaseToUint(easeVal));

    // spdlog::info("itdy: {}, ease {}", intervalDay, mapEaseToUint(easeVal));
    float easeFactor = computeProgress(easeVal, progress).easeFactor;
    return {.intervalDay = intervalDay,
            .dueDays = 0,
            .easeFactor = easeFactor};
}
