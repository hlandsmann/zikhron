#pragma once

#include <cstdint>

enum class EaseVal : unsigned int { again, hard, good, easy };

inline auto mapIntToEase(unsigned int e) -> EaseVal {
    switch (e) {
    case 0: return EaseVal::again;
    case 1: return EaseVal::hard;
    case 2: return EaseVal::good;
    case 3:
    default: return EaseVal::easy;
    }
}

inline auto mapEaseToUint(EaseVal e) -> unsigned {
    switch (e) {
    case EaseVal::again: return 0;
    case EaseVal::hard: return 1;
    case EaseVal::good: return 2;
    case EaseVal::easy: return 3;
    default: return 0;
    }
}

struct Ease {
    static constexpr float tempEaseFactorHard = 1.2F;
    static constexpr float minEaseFactor = 1.3F;
    static constexpr float maxEaseFactor = 2.5F;

    static constexpr float changeFactorEasy = 1.15F;
    static constexpr float changeFactorGood = 1.F;
    static constexpr float changeFactorHard = 0.85F;

    static constexpr float thresholdFactorGood = 2.1F;
    static constexpr float thresholdFactorHard = 1.6F;
    static constexpr float thresholdFactorAgain = minEaseFactor + 0.1F;

    static constexpr int thresholdIntervalHard = 5;
    static constexpr int thresholdIntervalAgain = 2;

    struct Progress {
        float intervalDay;
        float easeFactor;
    };
    Ease(float intervalDay, float easeFactor);
    [[nodiscard]] auto getProgress() const -> Progress;

    EaseVal easeVal;
    Progress progress;
};
