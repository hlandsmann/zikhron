#pragma once
#include "FileDialog.h"
#include "GroupAdd.h"
#include "GroupVideo.h"

#include <context/WidgetId.h>
#include <database/VideoPack.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spaced_repetition/DataBase.h>
#include <widgets/Layer.h>

#include <filesystem>
#include <kocoro/kocoro.hpp>
#include <memory>
#include <sigslot/signal.hpp>
#include <vector>

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

    template<class... Args>
    void connect_playVideoPack(Args&&... args)
    {
        sig_playVideoPack.connect(std::forward<Args>(args)...);
    }

private:
    auto manageVideosTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> kocoro::Task<>;
    std::shared_ptr<sr::DataBase> dataBase;
    std::shared_ptr<kocoro::SynchronousExecutor> executor;

    using GridPtr = std::shared_ptr<widget::Grid>;
    std::shared_ptr<kocoro::PersistentSignal<GridPtr>> signalGroupGrid;
    std::shared_ptr<kocoro::VolatileSignal<std::filesystem::path>> signalVideoFileOpen;

    constexpr static widget::BoxCfg gridCfg = {.padding = {},
                                               .paddingHorizontal = {},
                                               .paddingVertical = {},
                                               .border = 16.F};
    std::unique_ptr<FileDialog> fileDialog;
    std::vector<std::unique_ptr<GroupVideo>> groupVideos;
    std::unique_ptr<GroupAdd> groupAdd;
    context::WidgetId windowId{};

    sigslot::signal<database::VideoPackPtr> sig_playVideoPack;
};

} // namespace gui
