#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/ManualExecutor.h>
#include <folly/executors/thread_factory/NamedThreadFactory.h>
#include <folly/init/Init.h>
#include <gui/GlWindow.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spdlog/spdlog.h>

#include <boost/di.hpp>
#include <memory>
#include <thread>

namespace di = boost::di;

namespace {
auto createCPUThreadPoolExecutor() -> std::shared_ptr<folly::CPUThreadPoolExecutor>
{
    return std::make_shared<folly::CPUThreadPoolExecutor>(
            1, std::thread::hardware_concurrency(),
            std::make_shared<folly::NamedThreadFactory>("thread_pool"));
}
} // namespace

auto main(int argc, char** argv) -> int
{
    auto folly = folly::Init(&argc, &argv);

    auto injector = di::make_injector(di::bind<folly::CPUThreadPoolExecutor>().to(createCPUThreadPoolExecutor()));

    auto executor = injector.create<std::shared_ptr<folly::ManualExecutor>>();
    auto glWindow = injector.create<GlWindow>();
    while (!glWindow.shouldClose()) {
        glWindow.run();
        executor->run();
    }
}
