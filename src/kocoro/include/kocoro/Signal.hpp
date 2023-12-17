#pragma once
#include "ScheduleEntry.hpp"

#include <spdlog/spdlog.h>

#include <coroutine>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>
namespace kocoro {
template<class result_type>
struct SignalAwaiter
{
    using optional_handle = std::optional<std::coroutine_handle<>>;
    using optional_result = std::optional<result_type>;
    std::shared_ptr<optional_result> result;
    std::reference_wrapper<optional_handle> handle;
    std::reference_wrapper<std::mutex> resultMutex;

    SignalAwaiter(std::shared_ptr<optional_result> _result,
                  std::reference_wrapper<optional_handle> _handle,
                  std::reference_wrapper<std::mutex> _resultMutex)
        : result{std::move(_result)}
        , handle{_handle}
        , resultMutex{_resultMutex} {}
    [[nodiscard]] auto await_ready() const noexcept -> bool
    {
        auto lock = std::lock_guard{resultMutex.get()};
        return result->has_value();
    }

    void await_suspend(std::coroutine_handle<> _handle) noexcept
    {
        handle.get() = std::move(_handle);
    }

    [[nodiscard]] auto await_resume() const noexcept -> result_type
    {
        auto lock = std::lock_guard{resultMutex.get()};
        if (!result->has_value()) {
            return {};
        }
        return *std::exchange(*(result.get()), std::nullopt);
    }
};

template<class result_type>
class Signal : public ScheduleEntry
{
    using optional_result = std::optional<result_type>;
    using optional_handle = std::optional<std::coroutine_handle<>>;
    optional_handle handle;
    std::shared_ptr<optional_result> result{std::make_shared<optional_result>()};
    std::mutex resultMutex;

public:
    Signal() = default;
    auto operator co_await() -> SignalAwaiter<result_type>
    {
        return {result, std::ref(handle), std::ref(resultMutex)};
    }
    void set(result_type _result)
    {
        auto lock = std::lock_guard{resultMutex};
        result->emplace(std::move(_result));
    }
    auto resume() -> bool override
    {
        if (handle.has_value() && !handle->done() && result->has_value()) {
            auto tempHandle = std::exchange(handle, std::nullopt);
            tempHandle->resume();
            return true;
        }
        return false;
    }
};
} // namespace kocoro
