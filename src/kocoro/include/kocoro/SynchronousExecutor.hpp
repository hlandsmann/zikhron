#pragma once
#include "Async.hpp"
#include "ScheduleEntry.hpp"
#include "Signal.hpp"
#include "Task.hpp"

#include <spdlog/spdlog.h>

#include <memory>
#include <vector>
namespace kocoro {
class SynchronousExecutor
{
public:
    SynchronousExecutor() = default;

    template<class result_type>
    void startCoro(Task<result_type> task)
    {
        auto taskPtr = std::make_shared<Task<result_type>>();
        *taskPtr = std::move(task);
        taskPtr->resume();
        tasks.push_back(std::move(taskPtr));
    }

    template<class result_type>
    [[nodiscard]] auto makeVolatileSignal() -> std::shared_ptr<VolatileSignal<result_type>>
    {
        auto signalEntry = std::make_shared<VolatileSignal<result_type>>();
        scheduleEntries.push_back(signalEntry);
        return signalEntry;
    }

    template<class result_type>
    [[nodiscard]] auto makePersistentSignal() -> std::shared_ptr<PersistentSignal<result_type>>
    {
        auto signalEntry = std::make_shared<PersistentSignal<result_type>>();
        scheduleEntries.push_back(signalEntry);
        return signalEntry;
    }

    template<class result_type>
    [[nodiscard]] auto makeAsync() -> std::shared_ptr<Async<result_type>>
    {
        auto asyncEntry = std::make_shared<Async<result_type>>();
        scheduleEntries.push_back(asyncEntry);
        return asyncEntry;
    }
    auto run() -> bool;

private:
    std::vector<std::weak_ptr<ScheduleEntry>> scheduleEntries;
    std::vector<std::shared_ptr<TaskBase>> tasks;
};

} // namespace kocoro
