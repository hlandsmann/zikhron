#include "TabVideo.h"

#include <FileDialog.h>
#include <GroupAdd.h>
#include <imgui.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spdlog/spdlog.h>
#include <widgets/Layer.h>
#include <widgets/Window.h>

#include <filesystem>
#include <kocoro/kocoro.hpp>
#include <memory>
#include <utility>

namespace gui {
TabVideo::TabVideo(std::shared_ptr<kocoro::SynchronousExecutor> synchronousExecutor,
                   std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker)
    : executor{std::move(synchronousExecutor)}
    , signalVideoFileOpen{executor->makeVolatileSignal<std::filesystem::path>()}
{
    executor->startCoro(manageVideosTask(std::move(asyncTreeWalker)));
}

void TabVideo::setUp(std::shared_ptr<widget::Layer> layer)
{
    using namespace widget::layout;
    auto& cardWindow = *layer->add<widget::Window>(Align::start, width_expand, height_expand, "card_text");
    windowId = cardWindow.getWidgetId();
    auto grid = cardWindow.add<widget::Grid>(Align::start, gridCfg, 4, widget::Grid::Priorities{0.25F, 0.25F, 0.25F, 0.25F});
    groupAdd = std::make_unique<GroupAdd>(grid);
}

void TabVideo::displayOnLayer(widget::Layer& layer)
{
    auto window = layer.getWidget<widget::Window>(windowId);
    auto size = layer.getWidgetSize();
    auto droppedWindow = window.dropWindow();
    ;

    // open Dialog Simple
    if (groupAdd->draw()) {
        fileDialog = std::make_unique<FileDialog>(size,
                                                  "/home/harmen/Videos/chinesisch",
                                                  signalVideoFileOpen);
    }
    if (fileDialog) {
        fileDialog->draw();
    }
}

auto TabVideo::manageVideosTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> kocoro::Task<>
{
    dataBase = co_await asyncTreeWalker->getDataBase();
    while (true) {
        auto videoFile = co_await *signalVideoFileOpen;
        if (!videoFile.empty()) {
            dataBase->getVideoPackDB()->addVideoPack({videoFile});
        }
        fileDialog.reset();
    }
    co_return;
}

} // namespace gui
