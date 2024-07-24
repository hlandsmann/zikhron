#pragma once
#include <annotation/Ease.h>
#include <context/WidgetId.h>
#include <misc/Identifier.h>
#include <multimedia/MpvWrapper.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <widgets/Box.h>
#include <widgets/Layer.h>
#include <widgets/Video.h>
#include <widgets/Window.h>

#include <kocoro/kocoro.hpp>
#include <map>
#include <memory>

namespace gui {

class DisplayVideo
{
public:
    DisplayVideo(std::shared_ptr<kocoro::SynchronousExecutor> synchronousExecutor,
                 std::unique_ptr<multimedia::MpvWrapper> mpv);
    void setUp(widget::Layer& layer);
    void displayOnLayer(widget::Layer& layer);
    virtual ~DisplayVideo() = default;

    DisplayVideo(const DisplayVideo&) = delete;
    DisplayVideo(DisplayVideo&&) = delete;
    auto operator=(const DisplayVideo&) -> DisplayVideo& = delete;
    auto operator=(DisplayVideo&&) -> DisplayVideo& = delete;
    [[nodiscard]] auto getMpv() const -> std::shared_ptr<multimedia::MpvWrapper>;

private:
    using VocableId_Ease = std::map<VocableId, Ease>;
    auto feedingTask() -> kocoro::Task<>;

    std::shared_ptr<kocoro::SynchronousExecutor> executor;
    std::shared_ptr<multimedia::MpvWrapper> mpv;

    std::shared_ptr<widget::Video> video;
    std::shared_ptr<kocoro::VolatileSignal<bool>> signalShouldRender;
};

} // namespace gui
