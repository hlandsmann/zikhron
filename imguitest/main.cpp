#include <AsyncTreeWalker.h>
#include <MainWindow.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/ManualExecutor.h>
#include <folly/executors/thread_factory/NamedThreadFactory.h>
#include <folly/experimental/coro/Coroutine.h>
#include <folly/experimental/coro/Task.h>
#include <folly/futures/Future.h>
#include <folly/futures/SharedPromise.h>
#include <folly/init/Init.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <memory>
#include <thread>
#include <utility>

auto taskSlow45(folly::Future<int> f) -> folly::coro::Task<int>
{
    int i = co_await std::move(f);
    spdlog::info("Hello 45, i: {}", i);
    co_return 43;
}

auto taskSlow44() -> folly::coro::Task<int>
{
    spdlog::info("Hello 44");
    for (int i = 0; i < 10; i++) {
        spdlog::info("44, run: {}", i);
        co_await folly::futures::sleep(std::chrono::seconds{1});
    }
    co_return 43;
}

auto taskSlow43(std::shared_ptr<folly::ManualExecutor> ex) -> folly::coro::Task<int>
{
    taskSlow44().semi().via(ex.get());
    spdlog::info("hello 43");
    for (int i = 0; i < 10; i++) {
        spdlog::info("43, run: {}", i);
        co_await folly::futures::sleep(std::chrono::seconds{1});
    }
    co_return 43;
}

auto taskSlow42(std::shared_ptr<folly::CPUThreadPoolExecutor> cpuEx) -> folly::coro::Task<int>
{
    spdlog::info("hello 42");
    co_await taskSlow44().semi().via(cpuEx.get());
    for (int i = 0; i < 10; i++) {
        spdlog::info("42, run: {}", i);
        co_await folly::futures::sleep(std::chrono::seconds{1});
    }
    co_return 42;
}

auto main(int argc, char** argv) -> int
{
    auto folly = folly::Init(&argc, &argv);
    auto threadPoolExecutor = std::make_shared<folly::CPUThreadPoolExecutor>(
            1, std::thread::hardware_concurrency(),
            std::make_shared<folly::NamedThreadFactory>("thread_pool"));
    auto executor = std::make_shared<folly::ManualExecutor>();
    auto asyncTreeWalker = std::make_shared<AsyncTreeWalker>(executor, threadPoolExecutor);
    // folly::SharedPromise<int> p;
    // p.setWith([]() { return 168; });
    // auto f45 = p.getSemiFuture();
    // auto ts45 = taskSlow45(p.getFuture()).semi().via(executor.get());

    MainWindow mainWindow{executor, asyncTreeWalker};
    // auto task43 = taskSlow43;
    // auto f4 = taskSlow44().semi().via(threadPoolExecutor.get());
    // auto f2 = taskSlow42(threadPoolExecutor).semi().via(executor.get());
    // auto f3 = task43(executor).semi().via(executor.get());
    // future.then([](int x){spdlog::info("Value is {}", x);});
    while (!mainWindow.shouldClose()) {
        mainWindow.run();
        executor->run();
    }

    // spdlog::info("val: {}, {}", f2.
}

// Main code
