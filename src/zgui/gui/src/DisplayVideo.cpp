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
                           std::unique_ptr<multimedia::MpvWrapper> _mpv)
    : executor{std::move(_synchronousExecutor)}
    , mpv{std::move(_mpv)}
    , signalShouldRender{executor->makeVolatileSignal<bool>()}

{
    executor->startCoro(feedingTask());
}

auto DisplayVideo::getMpv() const -> std::shared_ptr<multimedia::MpvWrapper>
{
    return mpv;
}

void DisplayVideo::setUp(widget::Layer& layer)
{
    using namespace widget::layout;
    video = layer.add<widget::Video>(Align::start, mpv);
}

void DisplayVideo::displayOnLayer(widget::Layer& /* layer */)
{
    video->displayTexture();
}

auto DisplayVideo::feedingTask() -> kocoro::Task<>
{
    while (true) {
        const auto shouldRender = co_await mpv->SignalShouldRender();
        if (shouldRender) {
            video->render();
        }
    }
    co_return;
}

} // namespace gui
