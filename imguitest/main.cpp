#include <MainWindow.h>
#include <folly/executors/ManualExecutor.h>
#include <folly/executors/ThreadPoolExecutor.h>
#include <folly/executors/thread_factory/NamedThreadFactory.h>
#include <folly/experimental/coro/Coroutine.h>
#include <folly/experimental/coro/Task.h>
#include <folly/futures/Future.h>
#include <folly/init/Init.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <memory>

auto taskSlow43() -> folly::coro::Task<int>
{
    spdlog::info("Hello World");
    for (int i = 0; i < 3; i++) {
        spdlog::info("run: {}", i);
        co_await folly::futures::sleep(std::chrono::seconds{1});
    }
    co_return 43;
}

auto taskSlow42() -> folly::coro::Task<int>
{
    spdlog::info("Hello World");
    for (int i = 10; i < 13; i++) {
        spdlog::info("run: {}", i);
        co_await folly::futures::sleep(std::chrono::seconds{1});
    }
    co_return 42;
}

auto main(int argc, char** argv) -> int
{
    auto folly = folly::Init(&argc, &argv);
    auto executor = std::make_shared<folly::ManualExecutor>();
    // auto threadPoolExecutor = std::make_shared<folly::ThreadPoolExecutor>(
    //         1, 20, std::make_shared<folly::NamedThreadFactory>("thread_pool"));
    MainWindow mainWindow{executor};

    auto task = taskSlow43;
    auto f2 = taskSlow42().semi().via(executor.get());
    auto future = task().semi().via(executor.get());
    // future.then([](int x){spdlog::info("Value is {}", x);});
    while (!mainWindow.shouldClose()) {
        mainWindow.run();
        executor->run();
    }

    // spdlog::info("val: {}, {}", f2.
}

// Main code
