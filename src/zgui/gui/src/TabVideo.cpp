#include <TabVideo.h>
#include <context/imglog.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spdlog/spdlog.h>
#include <widgets/Box.h>
#include <widgets/Button.h>
#include <widgets/Grid.h>
#include <widgets/Layer.h>
#include <widgets/TextTokenSeq.h>
#include <widgets/Window.h>
#include <widgets/detail/Widget.h>

#include <kocoro/kocoro.hpp>
#include <memory>
#include <utility>

namespace gui {

TabVideo::TabVideo(std::shared_ptr<kocoro::SynchronousExecutor> _synchronousExecutor,
                   std::shared_ptr<sr::AsyncTreeWalker> _asyncTreeWalker)
    : executor{std::move(_synchronousExecutor)}
// , cardBoxContract{folly::makePromiseContract<std::shared_ptr<widget::Box>>()}

{
    // feedingTask(std::move(_asyncTreeWalker)).semi().via(synchronousExecutor.get());
    // signalVocIdEase->set(3);
    executor->startCoro(feedingTask(std::move(_asyncTreeWalker)));
}

void TabVideo::setUp(widget::Window& window)
{
    // using Align = widget::layout::Align;
    // auto& box = window.getBox();
    // box.setFlipChildrensOrientation(false);
    // auto grid = box.add<widget::Grid>(Align::start, 3);
    // auto layer = box.add<widget::Layer>(Align::start);
    // spdlog::info("setUp");
}

void TabVideo::displayOnWindow(widget::Window& window)
{
    // auto droppedWindow = window.dropWindow();
    //
    // auto& box = window.getBox();
    // // auto size = box.getExpandedSize();
    // // auto size = box.getWidgetSize();
    // // spdlog::critical("w: {}, h: {}, we: {}, he: {}", size.width, size.height, size.widthType, size.heightType);
    // box.start();
    // auto& grid = box.next<widget::Box>();
    // grid.start();
    // if (!grid.isLast()) {
    //     grid.next<widget::Button>().clicked();
    // }
    // imglog::log("width: {}, height: {}", window.getWidgetSize().width, window.getWidgetSize().height);
    // while (!cardBox.isLast()) {
    // cardBox.next<widget::Button>().clicked();
    // }
}

auto TabVideo::feedingTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> kocoro::Task<>
{
    using Align = widget::layout::Align;

    co_return;
}

} // namespace gui
