#include <CardDisplay.h>
#include <MainWindow.h>
#include <context/Fonts.h>
#include <context/Theme.h>
#include <imgui.h>
#include <widgets/Button.h>
#include <widgets/Widget.h>
#include <widgets/Window.h>

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
    // sideBar.doChoice({.x = rect.width - 80, .y = 0, .width = 80, .height = rect.height});
    // ImGui::PushFont(fonts->Gui());
    layout.arrange(rect);
    {
        auto droppedWindow = layout.next<widget::Window>().dropWindow();
        {
            auto chineseBig = theme->Font().dropChineseBig();
            ImGui::Text("位置");
            ImGui::Text("1");
        }
        // }
        // {
        //     auto droppedWindow = layout.next<widget::Window>().dropWindow();
        {
            auto chineseSmall = theme->Font().dropChineseSmall();
            ImGui::Text("位置");
            ImGui::Text("2");
        }
        // }
        // {
        //     auto droppedWindow = layout.next<widget::Window>().dropWindow();
        {
            auto gui = theme->Font().dropGui();
            ImGui::Text("Hello World");
            ImGui::Text("3");
        }
    }
    {
        auto& window = layout.next<widget::Window>();
        auto droppedWindow = window.dropWindow();
        window.getLayout().next<widget::Button>().clicked();
        window.getLayout().next<widget::Button>().clicked();
        window.getLayout().next<widget::Button>().clicked();

        // ImGui::PushFont(fonts->Gui());
        // ImGui::Text("Hello WOrld");
        // ImGui::Text("4");
        // ImGui::PopFont();
    }
    // ImGui::PopFont();
    bool show_demo_window = true;
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }
}

void MainWindow::arrangeLayout()
{
    using Align = widget::layout::Align;
    using namespace widget::layout;

    layout.add<widget::Window>(Align::start, width_expand, height_expand, "win1");
    // layout.add<widget::Window>(Align::center, 80, 0, width_fixed, height_expand, "win2");
    // layout.add<widget::Window>(Align::center, 80, 0, width_expand, height_expand, "win3");
    auto& window = *layout.add<widget::Window>(Align::end, width_fixed, height_expand, "win4");
    window.getLayout().add<widget::Button>(Align::start, "Cards");
    window.getLayout().add<widget::Button>(Align::start, "Video");
    window.getLayout().add<widget::Button>(Align::start, "Audio Group");
}
