#include "Scheduler2.h"

#include <algorithm>

namespace sr {

auto Scheduler2::nextIntervalDays(int lastInterval, int skew) -> int
{
    constexpr int defaultFactor = 2;
    constexpr double easeStep = 0.1;
    if (skew < skewMin || skew > skewMax) {
        return 0;
    }

    int newInterval = (lastInterval * defaultFactor) + skew;
    double factor = defaultFactor + (easeStep * skew);
    double dInterval = lastInterval * factor;

    if (skew < 0) {
        if (newInterval < lastInterval) {
            return 0;
        }
        return std::min(newInterval, static_cast<int>(dInterval));
    }
    if (skew > 0) {
        if (newInterval > lastInterval * 3) {
            return 0;
        }
        return std::max(newInterval, static_cast<int>(dInterval));
    }
    return newInterval;
}

auto Scheduler2::nextIntervalDays(int lastStableInterval, int lastInterval, int skew) -> int
{
    int newInterval = nextIntervalDays(lastInterval, skew);
    if (newInterval == lastStableInterval) {
        newInterval++;
    }

    return newInterval;
}
} // namespace sr
