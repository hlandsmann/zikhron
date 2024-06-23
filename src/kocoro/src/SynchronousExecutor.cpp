#include <utils/format.h>

#include <ScheduleEntry.hpp>
#include <SynchronousExecutor.hpp>
#include <memory>
#include <vector>

namespace kocoro {
auto SynchronousExecutor::run() -> bool
{
    auto erased = std::erase_if(scheduleEntries, [](const std::weak_ptr<ScheduleEntry>& scheduleEntry) {
        return scheduleEntry.expired();
    });
    if (erased != 0) {
        spdlog::warn("ScheduleEntries - {} erased", erased);
    }
    bool result = false;
    for (const auto& scheduleEntryWeak : scheduleEntries) {
        const auto& scheduleEntry = scheduleEntryWeak.lock();
        if (scheduleEntry == nullptr) {
            continue;
        }
        result |= scheduleEntry->resume();
    }
    return result;
}

} // namespace kocoro
