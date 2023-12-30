#include <CardDisplay.h>
#include <MainWindow.h>
#include <context/Fonts.h>
#include <context/Texture.h>
#include <context/Theme.h>
#include <context/imglog.h>
#include <imgui.h>
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
    , box{theme, widget::layout::Orientation::horizontal}
    , cardDisplay{std::move(_cardDisplay)}
{
}

void MainWindow::arrange(const widget::layout::Rect& _rect)
{
    arrangeDone &= (rect == _rect);
    rect = _rect;
    imglog::log("mainWindow arr, x {}, y {}, w{}, h{}", rect.x, rect.y, rect.width, rect.height);
    if (!arrangeDone) {
        box.arrange(rect);
    }
}

void MainWindow::doImGui()
{
    box.start();
    {
        auto& cardDisplayWindow = box.next<widget::Window>();
        cardDisplay->displayOnWindow(cardDisplayWindow);
    }
    {
        auto& tabWindow = box.next<widget::Window>();
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

    box.setPadding(0.F);
    auto& cardDisplayWin = *box.add<widget::Window>(Align::start, width_expand, height_expand, "cardDisplay");
    cardDisplay->setUp(cardDisplayWin);

    auto& window = *box.add<widget::Window>(Align::end, width_fixed, height_expand, "toggleButtonMenu");
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
