#include "DisplayVideo.h"

#include <context/imglog.h>
#include <multimedia/MpvWrapper.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spdlog/spdlog.h>
#include <widgets/Box.h>
#include <widgets/Button.h>
#include <widgets/Grid.h>
#include <widgets/ImageButton.h>
#include <widgets/Layer.h>
#include <widgets/MediaSlider.h>
#include <widgets/TextTokenSeq.h>
#include <widgets/Video.h>
#include <widgets/Window.h>
#include <widgets/detail/Widget.h>

#include <kocoro/kocoro.hpp>
#include <memory>
#include <utility>

namespace gui {

DisplayVideo::DisplayVideo(std::shared_ptr<kocoro::SynchronousExecutor> _synchronousExecutor,
                           std::shared_ptr<sr::AsyncTreeWalker> _asyncTreeWalker,
                           std::unique_ptr<multimedia::MpvWrapper> _mpv)
    : executor{std::move(_synchronousExecutor)}
    , mpv{std::move(_mpv)}
    , signalShouldRender{executor->makeVolatileSignal<bool>()}

{
    executor->startCoro(feedingTask(std::move(_asyncTreeWalker)));
}

void DisplayVideo::setUp(std::shared_ptr<widget::Layer> layer)
{
    using namespace widget::layout;
    auto box = layer->add<widget::Box>(Align::start, widget::Orientation::vertical);
    boxId = box->getWidgetId();
    box->setName("Video_box");
    auto& videoWindow = *box->add<widget::Window>(Align::start, width_expand, height_expand, "video_tex");
    video = videoWindow.add<widget::Video>(Align::start, mpv);

    auto& ctrlWindow = *box->add<widget::Window>(Align::end, ExpandType::width_expand, ExpandType::height_fixed, "card_ctrl");
    auto& ctrlBox = *ctrlWindow.add<widget::Box>(Align::start, widget::Orientation::horizontal);
    ctrlBox.setName("ctrlBox");
    ctrlBox.setPadding(0.F);
    ctrlBox.add<widget::ImageButton>(Align::start, context::Image::media_playback_start);
    ctrlBox.add<widget::MediaSlider>(Align::start);
    ctrlBox.add<widget::Button>(Align::center, "world");
}

void DisplayVideo::displayOnLayer(widget::Layer& layer)
{
    auto& box = layer.getWidget<widget::Box>(boxId);
    box.start();
    doVideoWindow(box.next<widget::Window>());
    doCtrlWindow(box.next<widget::Window>());
}

auto DisplayVideo::feedingTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> kocoro::Task<>
{
    using Align = widget::layout::Align;
    while (true) {
        const auto shouldRender = co_await mpv->SignalShouldRender();
        if (shouldRender) {
            video->render();
        }
    }
    co_return;
}

void DisplayVideo::doVideoWindow(widget::Window& videoWindow)
{
    auto droppedWindow = videoWindow.dropWindow();
    videoWindow.start();

    auto& video = videoWindow.next<widget::Video>();
    video.displayTexture();
}

void DisplayVideo::doCtrlWindow(widget::Window& ctrlWindow)
{
    auto droppedWindow = ctrlWindow.dropWindow();
}

} // namespace gui
