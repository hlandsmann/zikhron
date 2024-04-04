#pragma once
#include <annotation/Ease.h>
#include <context/WidgetIdGenerator.h>
#include <misc/Identifier.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <widgets/Box.h>
#include <widgets/Layer.h>
#include <widgets/Video.h>
#include <widgets/Window.h>

#include <kocoro/kocoro.hpp>
#include <map>
#include <memory>

namespace gui {

class TabVideo
{
public:
    TabVideo(std::shared_ptr<kocoro::SynchronousExecutor> synchronousExecutor,
             std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker,
             std::unique_ptr<multimedia::MpvWrapper> mpv);
    void setUp(std::shared_ptr<widget::Layer> layer);
    void displayOnLayer(widget::Layer& layer);
    virtual ~TabVideo() = default;

    TabVideo(const TabVideo&) = delete;
    TabVideo(TabVideo&&) = delete;
    auto operator=(const TabVideo&) -> TabVideo& = delete;
    auto operator=(TabVideo&&) -> TabVideo& = delete;

private:
    using VocableId_Ease = std::map<VocableId, Ease>;
    auto feedingTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> kocoro::Task<>;
    void doVideoWindow(widget::Window& videoWindow);
    void doCtrlWindow(widget::Window& ctrlWindow);

    std::shared_ptr<kocoro::SynchronousExecutor> executor;
    std::shared_ptr<multimedia::MpvWrapper> mpv;

    std::shared_ptr<widget::Video> video;
    std::shared_ptr<kocoro::VolatileSignal<bool>> signalShouldRender;
    context::WidgetId boxId{};
};

} // namespace gui
