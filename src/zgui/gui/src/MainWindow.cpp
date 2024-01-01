#include <CardDisplay.h>
#include <MainWindow.h>
#include <context/Fonts.h>
#include <context/Texture.h>
#include <context/Theme.h>
#include <context/imglog.h>
#include <imgui.h>
#include <widgets/Box.h>
#include <widgets/Button.h>
#include <widgets/ImageButton.h>
#include <widgets/ToggleButtonGroup.h>
#include <widgets/Widget.h>
#include <widgets/Window.h>

#include <initializer_list>
#include <memory>
#include <utility>

MainWindow::MainWindow(std::shared_ptr<context::Theme> _theme,
                       std::unique_ptr<CardDisplay> _cardDisplay)
    : theme{std::move(_theme)}
    , boxRect{std::make_shared<widget::layout::Rect>()}
    , box{std::make_shared<widget::Box>(widget::WidgetInit{
              .theme = theme,
              .rect = boxRect,
              .orientation = widget::layout::Orientation::horizontal,
              .align = widget::layout::Align::start,
              .parent = std::weak_ptr<widget::Widget>{}

      })}
    , cardDisplay{std::move(_cardDisplay)}
{
}

void MainWindow::arrange(const widget::layout::Rect& rect)
{
    needArrange |= (*boxRect != rect);
    needArrange |= box->arrangeIsNecessary();

    *boxRect = rect;
    imglog::log("mainWindow arr, x {}, y {}, w{}, h{}", rect.x, rect.y, rect.width, rect.height);
    if (needArrange) {
        needArrange = box->arrange();
    }
}

void MainWindow::doImGui()
{
    box->start();
    {
        auto& cardDisplayWindow = box->next<widget::Window>();
        cardDisplay->displayOnWindow(cardDisplayWindow);
    }
    {
        auto& tabWindow = box->next<widget::Window>();
        auto droppedWindow = tabWindow.dropWindow();
        // window.getLayout().next<widget::Button>().clicked();
        // window.getLayout().next<widget::Button>().clicked();
        // window.getLayout().next<widget::Button>().clicked();
        // window.getLayout().next<widget::ImageButton>().clicked();
        auto& tabBox = tabWindow.getBox();
        tabBox.start();
        tabBox.next<widget::ToggleButtonGroup>().getActive();
        tabBox.next<widget::ToggleButtonGroup>().getActive();
    }

    // bool show_demo_window = true;
    // if (show_demo_window) {
    //     // ImGui::SetNextWindowFocus();
    //     ImGui::ShowDemoWindow(&show_demo_window);
    // }
}

void MainWindow::setUp()
{
    using Align = widget::layout::Align;
    using namespace widget::layout;

    box->setPadding(0.F);
    auto& cardDisplayWin = *box->add<widget::Window>(Align::start, width_expand, height_expand, "cardDisplay");
    cardDisplay->setUp(cardDisplayWin);

    auto& window = *box->add<widget::Window>(Align::end, width_fixed, height_expand, "toggleButtonMenu");
    window.getBox().setOrientationVertical();
    window.getBox().setFlipChildrensOrientation(false);
    window.getBox().setPadding(0);

    window.getBox().add<widget::ToggleButtonGroup>(Align::start, std::initializer_list<context::Image>{
                                                                         context::Image::cards,
                                                                         context::Image::video,
                                                                         context::Image::audio,
                                                                         context::Image::configure});
    window.getBox().add<widget::ToggleButtonGroup>(Align::start, std::initializer_list<context::Image>{
                                                                         context::Image::cards,
                                                                         context::Image::video,
                                                                         context::Image::audio,
                                                                         context::Image::configure});
}
