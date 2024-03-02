#include <gui/GlWindow.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spdlog/spdlog.h>

#include <boost/di.hpp>
#include <kocoro/kocoro.hpp>

namespace di = boost::di;

auto main() -> int
{
    auto injector = di::make_injector();
    auto glWindow = injector.create<gui::GlWindow>();
    glWindow.run();
}
