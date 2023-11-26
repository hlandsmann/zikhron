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
                       CardDisplay _cardDisplay)
    : theme{std::move(_theme)}
    , cardDisplay{std::move(_cardDisplay)}
{
}

void MainWindow::doImGui(const widget::layout::Rect& rect)
{
    auto drop = context::Theme::dropImGuiStyleVars();
    layout.arrange(rect);
    {
        auto droppedWindow = layout.next<widget::Window>().dropWindow();
        {
            auto chineseBig = theme->getFont().dropChineseBig();
            ImGui::Text("位置");
            ImGui::Text("1");
        }
        {
            auto chineseSmall = theme->getFont().dropChineseSmall();
            ImGui::Text("位置");
            ImGui::Text("2");
        }
        {
            auto gui = theme->getFont().dropGui();
            ImGui::Text("Hello World");
            ImGui::Text("3");
        }
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
    bool show_demo_window = true;
    if (show_demo_window) {
        // ImGui::SetNextWindowFocus();
        ImGui::ShowDemoWindow(&show_demo_window);
    }
}

void MainWindow::arrangeLayout()
{
    using Align = widget::layout::Align;
    using namespace widget::layout;

    layout.setPadding(0.F);
    layout.add<widget::Window>(Align::start, width_expand, height_expand, "win1");
    // layout.add<widget::Window>(Align::center, 80, 0, width_fixed, height_expand, "win2");
    // layout.add<widget::Window>(Align::center, 80, 0, width_expand, height_expand, "win3");
    auto& window = *layout.add<widget::Window>(Align::end, width_fixed, height_expand, "win4");
    // window.getLayout().add<widget::Button>(Align::start, "Cards");
    // window.getLayout().add<widget::Button>(Align::start, "Video");
    // window.getLayout().add<widget::Button>(Align::start, "Audio Group");
    window.getLayout().setFlipChildrensOrientation(false);
    window.getLayout().add<widget::ToggleButtonGroup>(Align::start, std::initializer_list<context::Image>{
                                                                            context::Image::cards,
                                                                            context::Image::video,
                                                                            context::Image::audio,
                                                                            context::Image::configure});
}
