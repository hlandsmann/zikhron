#pragma once

namespace kocoro {
class ScheduleEntry
{
public:
    ScheduleEntry() = default;
    virtual ~ScheduleEntry() = default;
    ScheduleEntry(ScheduleEntry&&) = delete;
    ScheduleEntry(const ScheduleEntry&) = delete;
    auto operator=(ScheduleEntry&&) = delete;
    auto operator=(const ScheduleEntry&) = delete;
    virtual auto resume() -> bool = 0;
};

} // namespace kocoro
