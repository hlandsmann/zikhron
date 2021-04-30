#pragma once

enum class Ease : unsigned int { again, hard, good, easy };

inline auto mapIntToEase(unsigned int e) -> Ease {
    switch (e) {
    case 0: return Ease::again;
    case 1: return Ease::hard;
    case 2: return Ease::good;
    case 3: return Ease::easy;
    default: return Ease::easy;
    }
}
