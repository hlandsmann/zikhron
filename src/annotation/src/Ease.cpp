#include <Ease.h>
#include <algorithm>
#include <array>
#include <ranges>
#include <span>
#include <vector>

namespace ranges = std::ranges;

Ease::Ease(float intervalDay, float easeFactor, unsigned int indirectIntervalDay)
    : progress{.intervalDay = intervalDay,
               .easeFactor = easeFactor,
               .indirectIntervalDay = indirectIntervalDay} {
    ease = EaseVal::easy;
    if (easeFactor <= 2.1)
        ease = EaseVal::good;
    if (easeFactor <= 1.6 && intervalDay < 5)
        ease = EaseVal::hard;
    if (easeFactor <= 1.31 && intervalDay < 2)
        ease = EaseVal::again;
}

auto computeProgress(EaseVal ease, Ease::Progress progress) -> Ease::Progress {
    float easeChange = [=]() -> float {
        switch (ease) {
        case EaseVal::easy: return 1.15f;
        case EaseVal::good: return 1.0f;
        case EaseVal::hard: return 0.85f;
        default: return 0.f;
        }
    }();
    float easeFactor = std::clamp(std::max(1.3f, progress.easeFactor) * easeChange, 1.3f, 2.5f);

    float tempEaseFactor = [=]() -> float {
        switch (ease) {
        case EaseVal::easy: return easeFactor;
        case EaseVal::good: return easeFactor;
        case EaseVal::hard: return 1.2f;
        default: return 0.f;
        }
    }();

    float intervalDayExtra = [=]() -> float {
        switch (ease) {
        case EaseVal::easy: return float(progress.indirectIntervalDay) / 3.f;
        case EaseVal::good: return float(progress.indirectIntervalDay) / 4.f;
        default: return 0.f;
        }
    }();
    float intervalDay = progress.intervalDay * tempEaseFactor + intervalDayExtra;

    return {.intervalDay = intervalDay, .easeFactor = easeFactor, .indirectIntervalDay = 0};
}

auto Ease::getProgress() const -> Progress {
    std::array easeValList = {EaseVal::again, EaseVal::hard, EaseVal::good, EaseVal::easy};
    std::array<float, easeValList.size()> intervals;
    ranges::transform(easeValList, intervals.begin(), [this](EaseVal easeVal) {
        return computeProgress(easeVal, progress).intervalDay;
    });

    auto intervalSpan = std::span(std::next(intervals.begin()), intervals.end());
    for (auto it = intervalSpan.begin(); it < intervalSpan.end(); it++) {
        *it = std::max(*it, *std::prev(it) + 1.f);
    }

    float intervalDay = intervals[mapEaseToInt(ease)];
    float easeFactor = computeProgress(ease, progress).easeFactor;
    return {.intervalDay = intervalDay, .easeFactor = easeFactor, .indirectIntervalDay = 0};
}
