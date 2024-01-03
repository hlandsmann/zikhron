#include <Box.h>
#include <Window.h>
#include <context/Theme.h>
#include <context/imglog.h>
#include <detail/Widget.h>
#include <imgui.h>

#include <memory>
#include <string>
#include <utility>

namespace widget {
void Window::setup(layout::SizeType _sizeTypeWidth,
                   layout::SizeType _sizeTypeHeight,
                   std::string _name)
{
    boxRect = std::make_shared<layout::Rect>();
    // box = std::make_shared<Box>(getThemePtr(), PassiveOrientation(), std::weak_ptr{shared_from_this()});
    box = std::make_shared<Box>(WidgetInit{.theme = getThemePtr(),
                                           .widgetIdGenerator = getWidgetIdGenerator(),
                                           .rect = boxRect,
                                           .orientation = PassiveOrientation(),
                                           .align = layout::Align::start,
                                           .parent = std::weak_ptr{shared_from_this()}});
    sizeTypeWidth = _sizeTypeWidth;
    sizeTypeHeight = _sizeTypeHeight;
    name = std::move(_name);
}

Window::Window(const WidgetInit& init)
    : Widget{init}
{}

auto Window::dropWindow() -> WindowDrop
{
    layout::Rect rect = Rect();
    return {name, rect, getTheme().dropImGuiStyleColors(context::ColorTheme::Window)};
}

auto Window::arrange() -> bool
{
    *boxRect = Rect();
    boxRect->x = 0;
    boxRect->y = 0;
    return box->arrange();
}

auto Window::getBox() -> Box&
{
    return *box;
}

auto Window::calculateSize() const -> WidgetSize
{
    auto size = box->getWidgetSize();
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
