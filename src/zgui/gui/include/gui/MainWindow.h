#pragma once
#include "TabVideo.h"
#include "TabCard.h"

#include <context/GlfwImguiContext.h>
#include <context/Theme.h>
#include <context/WidgetId.h>
#include <widgets/Box.h>
#include <widgets/detail/Widget.h>

#include <cstddef>
#include <memory>

namespace gui {

class MainWindow
{
public:
    MainWindow(std::shared_ptr<context::Theme> theme,
               std::shared_ptr<context::WidgetIdGenerator> widgetIdGenerator,
               std::unique_ptr<TabCard> tabCard,
               std::unique_ptr<TabVideo> tabVideo);
    MainWindow(const MainWindow&) = delete;
    MainWindow(MainWindow&&) = delete;
    auto operator=(const MainWindow&) -> MainWindow& = delete;
    auto operator=(MainWindow&&) -> MainWindow& = delete;
    virtual ~MainWindow() = default;

    void arrange(const widget::layout::Rect& rect);
    void doImGui();
    void setup();

private:
    std::shared_ptr<context::Theme> theme;
    std::shared_ptr<widget::layout::Rect> boxRect;
    std::shared_ptr<widget::Box> box;
    std::unique_ptr<TabCard> tabCard;
    std::unique_ptr<TabVideo> tabVideo;
    bool needArrange = false;

    std::size_t activeTab{0};
};
} // namespace gui
