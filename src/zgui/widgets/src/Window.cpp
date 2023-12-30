#include "Box.h"
#include "Widget.h"

#include <Window.h>
#include <context/Theme.h>
#include <context/imglog.h>
#include <imgui.h>

#include <string>
#include <utility>

namespace widget {
Window::Window(const WidgetInit& init,
               layout::SizeType _sizeTypeWidth, layout::SizeType _sizeTypeHeight,
               std::string _name)
    : Widget<Window>{init}
    , box{init.theme, init.orientation, shared_from_this()}
    , sizeTypeWidth{_sizeTypeWidth}
    , sizeTypeHeight{_sizeTypeHeight}
    , name{std::move(_name)}
{}

auto Window::dropWindow() -> WindowDrop
{
    layout::Rect rect = Rect();
    return {name, rect, getTheme().dropImGuiStyleColors(context::ColorTheme::Window)};
}

auto Window::arrange() -> bool
{
    layout::Rect rect = Rect();
    layout::Rect layoutRect = {.x = 0, .y = 0, .width = rect.width, .height = rect.height};
    return box.arrange(layoutRect);
}

auto Window::getBox() -> Box&
{
    return box;
}

auto Window::calculateSize() const -> WidgetSize
{
    auto size = box.getWidgetSize();
    return {.widthType = sizeTypeWidth,
            .heightType = sizeTypeHeight,
            .width = size.width,
            .height = size.height};
}

WindowDrop::WindowDrop(const std::string& name, const widget::layout::Rect& rect,
                       context::StyleColorsDrop _styleColorsDrop)
    : styleColorsDrop{std::move(_styleColorsDrop)}
{
    ImGui::SetNextWindowPos({rect.x, rect.y});
    ImGui::SetNextWindowSize({rect.width, rect.height});
    ImGui::Begin(name.c_str(), nullptr,
                 ImGuiWindowFlags_NoTitleBar
                         | ImGuiWindowFlags_NoMove
                         | ImGuiWindowFlags_NoResize);
    incPopCount();
}

void WindowDrop::pop()
{
    ImGui::End();
}
} // namespace widget
