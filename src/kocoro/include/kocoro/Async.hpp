#pragma once
#include "ScheduleEntry.hpp"

#include <spdlog/spdlog.h>

#include <chrono>
#include <coroutine>
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <optional>
#include <utility>
namespace kocoro {

template<class result_type>
struct AsyncAwaiter
{
    using optional_handle = std::optional<std::coroutine_handle<>>;
    using optional_result = std::optional<result_type>;
    std::reference_wrapper<optional_result> result;
    std::reference_wrapper<optional_handle> handle;
    std::reference_wrapper<std::mutex> resultMutex;

    AsyncAwaiter(std::reference_wrapper<optional_result> _result,
                 std::reference_wrapper<optional_handle> _handle,
                 std::reference_wrapper<std::mutex> _resultMutex)
        : result{_result}
        , handle{_handle}
        , resultMutex{_resultMutex} {}
    [[nodiscard]] auto await_ready() const noexcept -> bool
    {
        return result.get().has_value();
    }

    void await_suspend(std::coroutine_handle<> _handle) noexcept
    {
        handle.get() = std::move(_handle);
    }

    [[nodiscard]] auto await_resume() const noexcept -> result_type
    {
        auto lock = std::lock_guard{resultMutex.get()};
        if (!result.get().has_value()) {
            return {};
        }
        return *(result.get());
    }
};

template<class result_type>
class Async : public ScheduleEntry
{
    using optional_result = std::optional<result_type>;
    using optional_handle = std::optional<std::coroutine_handle<>>;
    optional_handle handle;
    optional_result result;
    std::mutex resultMutex;
    std::future<result_type> future;
    std::function<result_type()> fun;

public:
    Async() = default;
    Async(std::function<result_type()> _fun)
        : fun{std::move(_fun)} {};
    void runAsync(std::function<result_type()> _fun) { fun = std::move(_fun); }
    auto operator co_await() -> AsyncAwaiter<result_type>
    {
        return {std::ref(result), std::ref(handle), std::ref(resultMutex)};
    }
    auto resume() -> bool override
    {
        progressFuture();
        if (handle.has_value() && !handle->done() && result.has_value()) {
            auto tempHandle = std::exchange(handle, std::nullopt);
            tempHandle->resume();
            return true;
        }
        return false;
    }
    void reset()
    {
        auto lock = std::lock_guard{resultMutex};
        result.reset();

        // this call may block (but it doesn't matter)
        future = std::future<result_type>{};
    }

private:
    void progressFuture()
    {
        auto lock = std::lock_guard{resultMutex};
        if (!result.has_value() && !future.valid() && fun) {
            future = std::async(fun);
        }
        if (future.valid() && future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            try {
                result = future.get();
            } catch (const std::exception& e) {
                spdlog::error("Async scheduleEntry error: {}", e.what());
            }
        }
    }
};

} // namespace kocoro
