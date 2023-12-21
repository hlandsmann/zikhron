#include <gui/GlWindow.h>
#include <kocoro/kocoro.hpp>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spdlog/spdlog.h>

#include <boost/di.hpp>
#include <memory>

namespace di = boost::di;

auto main() -> int
{
    auto injector = di::make_injector();

    auto executor = injector.create<std::shared_ptr<kocoro::SynchronousExecutor>>();
    auto glWindow = injector.create<GlWindow>();
    while (!glWindow.shouldClose()) {
        glWindow.run();
        executor->run();
    }
}
