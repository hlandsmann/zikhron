#include <CardDisplay.h>
#include <MainWindow.h>
#include <context/Fonts.h>
#include <context/Texture.h>
#include <context/Theme.h>
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
    , cardDisplay{std::move(_cardDisplay)}
{
}

void MainWindow::doImGui(int width, int height)
{
    widget::layout::Rect rect{0, 0, static_cast<float>(width), static_cast<float>(height)};
    auto drop = context::Theme::dropImGuiStyleVars();
    layout.arrange(rect);
    {
        auto& cardDisplayWindow = layout.next<widget::Window>();
        cardDisplay->displayOnWindow(cardDisplayWindow);
    }
    {
        auto& window = layout.next<widget::Window>();
        auto droppedWindow = window.dropWindow();
        // window.getLayout().next<widget::Button>().clicked();
        // window.getLayout().next<widget::Button>().clicked();
        // window.getLayout().next<widget::Button>().clicked();
        // window.getLayout().next<widget::ImageButton>().clicked();
        window.getLayout().next<widget::ToggleButtonGroup>().getActive();
    }

    // bool show_demo_window = true;
    // if (show_demo_window) {
    //     // ImGui::SetNextWindowFocus();
    //     ImGui::ShowDemoWindow(&show_demo_window);
    // }
}

void MainWindow::arrangeLayout()
{
    using Align = widget::layout::Align;
    using namespace widget::layout;

    layout.setPadding(0.F);
    auto& cardDisplayWin = *layout.add<widget::Window>(Align::start, width_expand, height_expand, "cardDisplay");
    cardDisplay->arrange(cardDisplayWin);

    auto& window = *layout.add<widget::Window>(Align::end, width_fixed, height_expand, "toggleButtonMenu");
    window.getLayout().setFlipChildrensOrientation(false);
    window.getLayout().add<widget::ToggleButtonGroup>(Align::start, std::initializer_list<context::Image>{
                                                                            context::Image::cards,
                                                                            context::Image::video,
                                                                            context::Image::audio,
                                                                            context::Image::configure});
}
