#pragma once
#include "ScheduleEntry.hpp"

#include <spdlog/spdlog.h>

#include <coroutine>
#include <functional>
namespace kocoro {

template<class result_type>
struct AsyncAwaiter
{
};

template<class result_type>
class Async : public ScheduleEntry
{
};

} // namespace kocoro
