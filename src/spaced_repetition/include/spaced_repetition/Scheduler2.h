#pragma once

namespace sr {

class Scheduler2
{
public:
    Scheduler2() = default;
    [[nodiscard]] static auto nextIntervalDays(int lastInterval, int skew) -> int;
    [[nodiscard]] static auto nextIntervalDays(int lastStableInterval, int lastInterval, int skew) -> int;

private:
    static constexpr int skewMin = -7;
    static constexpr int skewMax = 10;

    static constexpr int difficultyMin = 0;
    static constexpr int difficultyMax = 10;
    static constexpr int difficultyDefault = 2;
};

} // namespace sr
