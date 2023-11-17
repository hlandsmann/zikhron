#include <AsyncTreeWalker.h>
#include <MainWindow.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/ManualExecutor.h>
#include <folly/executors/thread_factory/NamedThreadFactory.h>
#include <folly/init/Init.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <thread>

auto main(int argc, char** argv) -> int
{
    auto folly = folly::Init(&argc, &argv);

    auto executor = std::make_shared<folly::ManualExecutor>();
    auto threadPoolExecutor = std::make_shared<folly::CPUThreadPoolExecutor>(
            1, std::thread::hardware_concurrency(),
            std::make_shared<folly::NamedThreadFactory>("thread_pool"));

    auto asyncTreeWalker = std::make_shared<AsyncTreeWalker>(executor, threadPoolExecutor);

    MainWindow mainWindow{executor, asyncTreeWalker};
    while (!mainWindow.shouldClose()) {
        mainWindow.run();
        executor->run();
    }
}
