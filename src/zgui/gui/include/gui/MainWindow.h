#pragma once
#include "CardDisplay.h"

#include <context/GlfwImguiContext.h>
#include <context/Theme.h>
#include <context/WidgetIdGenerator.h>
#include <widgets/Box.h>
#include <widgets/detail/Widget.h>

#include <memory>

class MainWindow
{
public:
    MainWindow(std::shared_ptr<context::Theme> theme,
               std::shared_ptr<context::WidgetIdGenerator> widgetIdGenerator,
               std::unique_ptr<CardDisplay> cardDisplay);

    void arrange(const widget::layout::Rect& rect);
    void doImGui();
    void setUp();

private:
    std::shared_ptr<context::Theme> theme;
    std::shared_ptr<widget::layout::Rect> boxRect;
    std::shared_ptr<widget::Box> box;
    std::unique_ptr<CardDisplay> cardDisplay;
    bool needArrange = false;
};
