#include "TabVideo.h"

#include <FileDialog.h>
#include <GroupAdd.h>
#include <GroupVideo.h>
#include <imgui.h>
#include <misc/Language.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spdlog/spdlog.h>
#include <theme/Sizes.h>
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
    , signalGroupGrid{executor->makePersistentSignal<GridPtr>()}
    , signalVideoFileOpen{executor->makeVolatileSignal<std::filesystem::path>()}
    , signalLanguage{executor->makePersistentSignal<Language>()}
{
    executor->startCoro(manageVideosTask(std::move(asyncTreeWalker)));
}

void TabVideo::setUp(widget::Layer& layer)
{
    using namespace widget::layout;
    auto& cardWindow = *layer.add<widget::Window>(Align::start, width_expand, height_expand, "card_text");
    windowId = cardWindow.getWidgetId();
    auto grid = cardWindow.add<widget::Grid>(Align::start, gridCfg, 1, widget::Grid::Priorities{1.0F});
    grid->setExpandType(width_expand, height_expand);
    grid->setVerticalPadding(20.F);
    signalGroupGrid->set(grid);
}

void TabVideo::setLanguage(Language language)
{
    signalLanguage->set(language);
}

void TabVideo::displayOnLayer(widget::Layer& layer)
{
    auto& window = layer.getWidget<widget::Window>(windowId);
    auto size = layer.getWidgetSize();
    auto droppedWindow = window.dropWindow();

    // open Dialog Simple
    if (groupAdd && groupAdd->draw()) {
        fileDialog = std::make_unique<FileDialog>(size,
                                                  "/home/harmen/Videos/chinesisch",
                                                  signalVideoFileOpen);
    }
    for (const auto& groupVideo : groupVideos) {
        if (groupVideo->draw()) {
            spdlog::info("Clicked: {}", groupVideo->getVideoSet()->getName());
            sig_playVideoSet(groupVideo->getVideoSet());
        }
    }
    if (fileDialog) {
        fileDialog->draw();
    }

    window.start();
    auto& grid = window.next<widget::Grid>();
    grid.autoSetColumnsPaddingRearrange(Sizes::groupMinPadding, Sizes::groupMaxPadding);
}

auto TabVideo::manageVideosTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> kocoro::Task<>
{
    auto language = co_await *signalLanguage;
    dataBase = co_await asyncTreeWalker->getDataBase(language);
    auto groupGrid = co_await *signalGroupGrid;
    auto videoDB = dataBase->getCardDB()->getVideoDB();
    while (true) {
        groupGrid->clear();
        groupVideos.clear();
        for (const auto& videoSet : videoDB->getVideoSets()) {
            groupVideos.push_back(std::make_unique<GroupVideo>(groupGrid, videoSet));
        }
        groupAdd = std::make_unique<GroupAdd>(groupGrid);

        auto videoFile = co_await *signalVideoFileOpen;
        if (!videoFile.empty()) {
            videoDB->addVideoSet({videoFile});
        }
        fileDialog.reset();
    }
    co_return;
}

} // namespace gui
