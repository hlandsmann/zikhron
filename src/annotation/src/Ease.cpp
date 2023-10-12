#include <Ease.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <span>

namespace ranges = std::ranges;

Ease::Ease(float intervalDay, float easeFactor, int indirectIntervalDay)
    : easeVal{EaseVal::easy}
    , progress{.intervalDay = intervalDay,
               .easeFactor = easeFactor,
               .indirectIntervalDay = indirectIntervalDay}
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

    float intervalDayExtra = [=]() -> float {
        constexpr static float divisorExtraEasy = 3.F;
        constexpr static float divisorExtraGood = 4.F;
        switch (ease) {
        case EaseVal::easy:
            return float(progress.indirectIntervalDay) / divisorExtraEasy;
        case EaseVal::good:
            return float(progress.indirectIntervalDay) / divisorExtraGood;
        default:
            return 0.F;
        }
    }();
    float intervalDay = progress.intervalDay * tempEaseFactor + intervalDayExtra;
    spdlog::info("itdy: {}", intervalDay);
    return {.intervalDay = intervalDay, .easeFactor = easeFactor, .indirectIntervalDay = 0};
}

auto Ease::getProgress() const -> Progress
{
    std::array easeValList = {EaseVal::again, EaseVal::hard, EaseVal::good, EaseVal::easy};
    std::array<float, easeValList.size()> intervals{};
    ranges::transform(easeValList, intervals.begin(), [this](EaseVal easeVal) {
        return computeProgress(easeVal, progress).intervalDay;
    });

    auto intervalSpan = std::span(std::next(intervals.begin()), intervals.end());
    for (auto it = intervalSpan.begin(); it < intervalSpan.end(); it++) {
        *it = std::max(*it, *std::prev(it) + 1.F);
    }

    float intervalDay = intervals.at(mapEaseToUint(easeVal));

    spdlog::info("itdy: {}, ease {}", intervalDay, mapEaseToUint(easeVal));
    float easeFactor = computeProgress(easeVal, progress).easeFactor;
    return {.intervalDay = intervalDay, .easeFactor = easeFactor, .indirectIntervalDay = 0};
}
