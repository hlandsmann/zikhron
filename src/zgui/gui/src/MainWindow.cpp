#include "MainWindow.h"

#include <DisplayCard.h>
#include <DisplayVideo.h>
#include <context/Fonts.h>
#include <context/Texture.h>
#include <context/Theme.h>
#include <context/WidgetIdGenerator.h>
#include <context/imglog.h>
#include <imgui.h>
#include <widgets/Box.h>
#include <widgets/Button.h>
#include <widgets/ImageButton.h>
#include <widgets/Layer.h>
#include <widgets/ToggleButtonGroup.h>
#include <widgets/Window.h>
#include <widgets/detail/Widget.h>

#include <initializer_list>
#include <memory>
#include <utility>

MainWindow::MainWindow(std::shared_ptr<context::Theme> _theme,
                       std::shared_ptr<context::WidgetIdGenerator> widgetIdGenerator,
                       std::unique_ptr<DisplayCard> _displayCard,
                       std::unique_ptr<DisplayVideo> _displayVideo)
    : theme{std::move(_theme)}
    , boxRect{std::make_shared<widget::layout::Rect>()}
    , box{std::make_shared<widget::Box>(widget::WidgetInit{
              .theme = theme,
              .widgetIdGenerator = std::move(widgetIdGenerator),
              .rect = boxRect,
              .horizontalAlign = widget::layout::Align::start,
              .verticalAlign = widget::layout::Align::start,
              .parent = std::weak_ptr<widget::Widget>{}

      })}
    , displayCard{std::move(_displayCard)}
    , displayVideo{std::move(_displayVideo)}
{
}

void MainWindow::arrange(const widget::layout::Rect& rect)
{
    needArrange |= (*boxRect != rect);
    needArrange |= box->arrangeIsNecessary();

    *boxRect = rect;
    imglog::log("mainWindow arr, x {}, y {}, w{}, h{}", rect.x, rect.y, rect.width, rect.height);
    // if (needArrange) {
    needArrange = box->arrange(rect);
    // }
}

void MainWindow::doImGui()
{
    box->start();
    {
        auto layer = box->next<widget::Layer>();
        displayCard->displayOnLayer(layer);
        // auto& displayWindow = box->next<widget::Window>();
        // displayCard->displayOnWindow(displayWindow);
        // auto drop = displayWindow.dropWindow();
    }
    {
        auto& tabWindow = box->next<widget::Window>();
        auto droppedWindow = tabWindow.dropWindow();
        // window.getLayout().next<widget::Button>().clicked();
        // window.getLayout().next<widget::Button>().clicked();
        // window.getLayout().next<widget::Button>().clicked();
        // window.getLayout().next<widget::ImageButton>().clicked();
        tabWindow.start();
        auto& tabBox = tabWindow.next<widget::Box>();
        tabBox.start();
        tabBox.next<widget::ToggleButtonGroup>().getActive();
        // tabBox.next<widget::ToggleButtonGroup>().getActive();
    }

    // bool show_demo_window = true;
    // if (show_demo_window) {
    //     // ImGui::SetNextWindowFocus();
    //     ImGui::ShowDemoWindow(&show_demo_window);
    // }
}

void MainWindow::setup()
{
    using Align = widget::layout::Align;
    using namespace widget::layout;

    box->setPadding(0.F);
    auto mainLayer = box->add<widget::Layer>(Align::start);
    mainLayer->setExpandType(width_expand, height_expand);
    mainLayer->setName("mainLayer");

    // auto& displayWindow = *box->add<widget::Window>(Align::start, width_expand, height_expand, "cardDisplay");
    displayCard->setUp(mainLayer);

    auto& toggleButtonMenu = *box->add<widget::Window>(Align::end, width_fixed, height_expand, "toggleButtonMenu");
    auto& tmbBox = *toggleButtonMenu.add<widget::Box>(Align::start, widget::Orientation::vertical);

    // tmbBox->setFlipChildrensOrientation(false);
    tmbBox.setPadding(0);

    tmbBox.add<widget::ToggleButtonGroup>(Align::start, widget::Orientation::vertical,
                                          std::initializer_list<context::Image>{
                                                  context::Image::cards,
                                                  context::Image::video,
                                                  context::Image::audio,
                                                  context::Image::configure});
}
