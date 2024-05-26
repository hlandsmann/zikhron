#include <Ease.h>
// #include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <span>

namespace ranges = std::ranges;

Ease::Ease(float intervalDay, int dueDays, float easeFactor)
    : easeVal{EaseVal::easy}
    , progress{.intervalDay = intervalDay,
               .dueDays = dueDays,
               .easeFactor = easeFactor}
{
    if (easeFactor <= thresholdFactorGood) {
        easeVal = EaseVal::good;
    }
    if (easeFactor <= thresholdFactorHard && intervalDay < thresholdIntervalHard) {
        easeVal = EaseVal::hard;
    }
    if (easeFactor <= thresholdFactorAgain && intervalDay < thresholdIntervalAgain) {
        easeVal = EaseVal::again;
    }
}

auto computeProgress(EaseVal ease, Ease::Progress progress) -> Ease::Progress
{
    float easeChange = [=]() -> float {
        switch (ease) {
        case EaseVal::easy:
            return Ease::changeFactorEasy;
        case EaseVal::good:
            return Ease::changeFactorGood;
        case EaseVal::hard:
            return Ease::changeFactorHard;
        default:
            return 0.F;
        }
    }();
    float easeFactor = std::clamp(std::max(Ease::minEaseFactor, progress.easeFactor) * easeChange,
                                  Ease::minEaseFactor,
                                  Ease::maxEaseFactor);

    float tempEaseFactor = [=]() -> float {
        switch (ease) {
        case EaseVal::easy:
        case EaseVal::good:
            return easeFactor;
        case EaseVal::hard:
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
    // spdlog::info("itdy: {}", intervalDay);
    return {.intervalDay = intervalDay,
            .dueDays = 0,
            .easeFactor = easeFactor};
}

auto Ease::getProgress() const -> Progress
{
    std::array easeValList = {EaseVal::again, EaseVal::hard, EaseVal::good, EaseVal::easy};
    std::array<float, easeValList.size()> intervals{};
    ranges::transform(easeValList, intervals.begin(), [this](EaseVal ease) {
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
