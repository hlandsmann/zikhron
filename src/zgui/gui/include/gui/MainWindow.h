#pragma once
#include "CardDisplay.h"
#include <widgets/Box.h>
#include <context/GlfwImguiContext.h>
#include <context/Fonts.h>
#include <widgets/Widget.h>

#include <memory>

class MainWindow
{
public:
    MainWindow(CardDisplay cardDisplay);

    void doImGui(const widget::layout::Rect& rect);
    void arrangeLayout();

private:
    std::unique_ptr<Fonts> fonts;
    std::shared_ptr<widget::Theme> theme;
    widget::Box layout{theme, widget::layout::Orientation::horizontal};
    CardDisplay cardDisplay;
};
