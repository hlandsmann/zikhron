#include <gui/GlWindow.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <utils/ProcessPipe.h>
#include <utils/format.h>

#include <boost/di.hpp>
#include <kocoro/kocoro.hpp>
#include <memory>
#include <string>
#include <vector>

#include <unistd.h>

namespace di = boost::di;

static auto getProcessPipe() -> std::shared_ptr<utl::ProcessPipe>
{
    auto _ = chdir("/home/harmen/zikhron/sudachi-0.7.5/");
    return std::make_shared<utl::ProcessPipe>("/usr/bin/java", std::vector<std::string>{"-jar", "sudachi-0.7.5.jar", "-m", "C", "-a"});
}

auto main() -> int
{
    spdlog::set_level(spdlog::level::trace);
    auto injector = di::make_injector(boost::di::bind<utl::ProcessPipe>.to(getProcessPipe()));
    injector.create<std::shared_ptr<utl::ProcessPipe>>();
    auto glWindow = injector.create<std::shared_ptr<gui::GlWindow>>();
    glWindow->run();
}
