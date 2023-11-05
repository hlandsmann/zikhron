#include <widget/Layout.h>
#include <SideBar.h>
#include <imgui.h>

auto SideBar::doChoice(const widget::layout::rect& rect) -> Tab
{
    ImGui::SetNextWindowPos({rect.x, rect.y});
    ImGui::SetNextWindowSize({rect.width, rect.height});

    ImGui::Begin("Window Name", nullptr,
                 ImGuiWindowFlags_NoTitleBar
                         | ImGuiWindowFlags_NoMove
                         | ImGuiWindowFlags_NoResize);

    ImGui::End();
    return Tab::TextCards;
}
