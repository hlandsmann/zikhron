#pragma once
#include "CardDisplay.h"

#include <context/GlfwImguiContext.h>
#include <context/Theme.h>
#include <widgets/Box.h>
#include <widgets/Widget.h>

#include <memory>

class MainWindow
{
public:
    MainWindow(std::shared_ptr<context::Theme> theme,
               CardDisplay cardDisplay);

    void doImGui(const widget::layout::Rect& rect);
    void arrangeLayout();

private:
    std::shared_ptr<context::Theme> theme;
    widget::Box layout{theme, widget::layout::Orientation::horizontal};
    CardDisplay cardDisplay;
};
