#pragma once
#include "ScheduleEntry.hpp"

#include <spdlog/spdlog.h>

#include <chrono>
#include <coroutine>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>

namespace kocoro {
template<class result_type, bool persistent>
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
        if constexpr (persistent) {
            return result.get()->value();
        } else {
            return *std::exchange(*(result.get()), std::nullopt);
        }
    }
};

template<class result_type, bool persistent>
class Signal : public ScheduleEntry
{
    using optional_result = std::optional<result_type>;
    using optional_handle = std::optional<std::coroutine_handle<>>;
    optional_handle handle;
    std::shared_ptr<optional_result> result{std::make_shared<optional_result>()};
    std::mutex resultMutex;

    using time_point = std::chrono::time_point<std::chrono::system_clock>;
    std::optional<time_point> timeout;

public:
    Signal() = default;

    auto operator co_await() -> SignalAwaiter<result_type, persistent>
    {
        return {result, std::ref(handle), std::ref(resultMutex)};
    }

    void set(result_type _result)
    {
        auto lock = std::lock_guard{resultMutex};
        result->emplace(std::move(_result));
        timeout.reset();
    }

    void reset()
    {
        auto lock = std::lock_guard{resultMutex};
        result->reset();
        timeout.reset();
    }

    auto resume() -> bool override
    {
        if (handle.has_value() && !handle->done() && (result->has_value() || isTimeOut())) {
            auto tempHandle = std::exchange(handle, std::nullopt);
            tempHandle->resume();
            return true;
        }
        return false;
    }

    void setTimeOut(std::chrono::milliseconds ms)
    {
        timeout = std::chrono::system_clock::now() + ms;
    }

    auto isTimeOut() -> bool
    {
        if (!timeout.has_value()) {
            return false;
        }
        return timeout < std::chrono::system_clock::now();
    }
};

template<class result_type>
using VolatileSignal = Signal<result_type, false>;

template<class result_type>
using PersistentSignal = Signal<result_type, true>;

} // namespace kocoro
