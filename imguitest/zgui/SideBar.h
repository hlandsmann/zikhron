#pragma once
#include <imgui.h>
#include <widget/Box.h>

class SideBar
{
public:
    SideBar() = default;

    enum class Tab {
        TextCards,
        Video,
        SupplyAudio,
    };

    auto doChoice(const widget::layout::Rect& rect) -> Tab;

private:
    constexpr static float width = 40.F;
    int choice{0};
};
