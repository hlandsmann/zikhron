#pragma once

enum class EaseVal : unsigned int { again, hard, good, easy };

inline auto mapIntToEase(unsigned int e) -> EaseVal {
    switch (e) {
    case 0: return EaseVal::again;
    case 1: return EaseVal::hard;
    case 2: return EaseVal::good;
    case 3: return EaseVal::easy;
    default: return EaseVal::easy;
    }
}

inline auto mapEaseToInt(EaseVal e) -> int {
    switch (e) {
    case EaseVal::again: return 0;
    case EaseVal::hard: return 1;
    case EaseVal::good: return 2;
    case EaseVal::easy: return 3;
    default: return 0;
    }
}

struct Ease {
    struct Progress {
        float intervalDay;
        float easeFactor;
        unsigned int indirectIntervalDay;
    };
    Ease(float intervalDay, float easeFactor, unsigned int indirectIntervalDay);
    auto getProgress() const -> Progress;

    EaseVal ease;
    Progress progress;
};