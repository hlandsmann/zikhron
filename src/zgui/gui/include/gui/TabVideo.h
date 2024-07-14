#pragma once
#include "FileDialog.h"
#include "GroupAdd.h"

#include <context/WidgetId.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spaced_repetition/DataBase.h>
#include <widgets/Layer.h>

#include <filesystem>
#include <kocoro/kocoro.hpp>
#include <memory>

namespace gui {
class TabVideo
{
public:
    TabVideo(std::shared_ptr<kocoro::SynchronousExecutor> synchronousExecutor,
             std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker);
    TabVideo(const TabVideo&) = delete;
    TabVideo(TabVideo&&) = delete;
    auto operator=(const TabVideo&) -> TabVideo& = delete;
    auto operator=(TabVideo&&) -> TabVideo& = delete;
    virtual ~TabVideo() = default;

    void setUp(std::shared_ptr<widget::Layer> layer);
    void displayOnLayer(widget::Layer& layer);

private:
    auto manageVideosTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> kocoro::Task<>;
    std::shared_ptr<sr::DataBase> dataBase;
    std::shared_ptr<kocoro::SynchronousExecutor> executor;
    std::shared_ptr<kocoro::VolatileSignal<std::filesystem::path>> signalVideoFileOpen;

    constexpr static widget::BoxCfg gridCfg = {.padding = {},
                                               .paddingHorizontal = {},
                                               .paddingVertical = {},
                                               .border = 16.F};
    std::unique_ptr<FileDialog> fileDialog;
    std::unique_ptr<GroupAdd> groupAdd;
    context::WidgetId windowId{};
};

} // namespace gui
