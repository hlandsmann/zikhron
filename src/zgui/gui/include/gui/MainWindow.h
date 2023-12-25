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
               std::unique_ptr<CardDisplay> cardDisplay);

    void doImGui(int width, int height);
    void arrangeLayout();

private:
    std::shared_ptr<context::Theme> theme;
    widget::Box layout{theme, widget::layout::Orientation::horizontal};
    std::unique_ptr<CardDisplay> cardDisplay;
};
