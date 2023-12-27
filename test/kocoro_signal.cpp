#include <spdlog/spdlog.h>

#include <kocoro/kocoro.hpp>
#include <memory>

#include <unistd.h>

auto subcoro(std::shared_ptr<kocoro::VolatileSignal<int>> signal0)
        -> kocoro::Task<int>
{
    int val{};
    val = co_await *signal0;
    spdlog::info("subcoro signal0: {}", val);
    co_return val;
}

auto coro_await_signal(std::shared_ptr<kocoro::VolatileSignal<int>> signal0,
                       std::shared_ptr<kocoro::VolatileSignal<int>> signal1)
        -> kocoro::Task<void>
{
    spdlog::info("coro_await_signal");
    int val{};
    for (int i = 0; i < 5; i++) {
        val = co_await subcoro(signal0);
        spdlog::info("signal0: {}", val);

        val = co_await *signal1;
        spdlog::info("signal1: {}", val);
    }
    co_return;
}

auto coro_await_async(std::shared_ptr<kocoro::Async<int>> async0) -> kocoro::Task<void>
{
    spdlog::info("coro_await_async");
    int val{};
    val = co_await *async0;
    spdlog::info("async0: {}", val);
    val = co_await *async0;
    spdlog::info("async0: {}", val);

    spdlog::info("async reset..");
    async0->reset();
    val = co_await *async0;
    spdlog::info("async0: {}", val);
    co_return;
}

auto main() -> int
{
    spdlog::info("kocoro_main");
    kocoro::SynchronousExecutor synchronousExecutor;
    auto signal0 = synchronousExecutor.makeVolatileSignal<int>();
    auto signal1 = synchronousExecutor.makeVolatileSignal<int>();
    auto async0 = synchronousExecutor.makeAsync<int>();
    synchronousExecutor.startCoro(coro_await_signal(signal0, signal1));
    synchronousExecutor.startCoro(coro_await_async(async0));
    async0->runAsync([]() {
        spdlog::info("async start...");
        sleep(1);
        spdlog::info("async stop...");
        return 1;
    });
    // signal0->set(42);
    // spdlog::info("main");
    // auto handle = coro_await_signal(signal0, signal1);
    // handle.resume();
    // signal0->set(1);
    // signal0->set(2);
    // signal0->set(3);
    // signal0->set(4);
    // spdlog::info("set event0");
    // signal1->set(5);

    for (int i = 0; i < 20; i++) {
        if (i % 2 == 0) {
            signal0->set(i);
        } else {
            signal1->set(i);
        }
        synchronousExecutor.run();
    }
    while (true) {
        synchronousExecutor.run();
    }
    return 0;
}
