#include <gui/GlWindow.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <utils/spdlog.h>

#include <boost/di.hpp>
#include <kocoro/kocoro.hpp>
#include <memory>

namespace di = boost::di;

auto main() -> int
{
    spdlog::set_level(spdlog::level::trace);
    auto injector = di::make_injector();
    auto glWindow = injector.create<std::shared_ptr<gui::GlWindow>>();
    glWindow->run();
}
