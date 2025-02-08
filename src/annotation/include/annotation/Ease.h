#pragma once

#include <utility>

enum class Rating : unsigned int {
    again,
    hard,
    good,
    easy
};

inline auto mapIntToEase(unsigned int e) -> Rating
{
    switch (e) {
    case 0:
        return Rating::again;
    case 1:
        return Rating::hard;
    case 2:
        return Rating::good;
    case 3:
    default:
        return Rating::easy;
    }
}

inline auto mapEaseToUint(Rating e) -> unsigned
{
    switch (e) {
    case Rating::again:
        return 0;
    case Rating::hard:
        return 1;
    case Rating::good:
        return 2;
    case Rating::easy:
        return 3;
    }
    std::unreachable();
}

struct Ease
{
    static constexpr float tempEaseFactorHard = 1.2F;
    static constexpr float minEaseFactor = 1.3F;
    static constexpr float maxEaseFactor = 2.5F;

    static constexpr float changeFactorEasy = 1.15F;
    static constexpr float changeFactorGood = 1.F;
    static constexpr float changeFactorHard = 0.85F;

    static constexpr float thresholdFactorGood = 2.2F;
    static constexpr float thresholdFactorHard = 1.4F;
    static constexpr float thresholdFactorAgain = minEaseFactor + 0.1F;

    static constexpr int thresholdIntervalHard = 3;
    static constexpr int thresholdIntervalAgain = 1;

    struct Progress
    {
        float intervalDay;
        int dueDays;
        float easeFactor;
    };

    Ease(float intervalDay, int dueDays, float easeFactor);
    [[nodiscard]] auto getProgress() const -> Progress;

    Rating easeVal;
    Progress progress;
};
