#include <Box.h>
#include <Layer.h>
#include <Window.h>
#include <context/Theme.h>
#include <context/imglog.h>
#include <detail/Widget.h>
#include <imgui.h>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>

namespace widget {
void Window::setup(layout::ExpandType _expandTypeWidth,
                   layout::ExpandType _expandTypeHeight,
                   std::string _name)
{
    layer = std::make_shared<widget::Layer>(widget::WidgetInit{
            .theme = getThemePtr(),
            .widgetIdGenerator = getWidgetIdGenerator(),
            .rect = std::make_shared<layout::Rect>(),
            .orientation = widget::layout::Orientation::horizontal,
            .horizontalAlign = widget::layout::Align::start,
            .verticalAlign = widget::layout::Align::start,
            .parent = std::weak_ptr<widget::Widget>{}

    });
    expandTypeWidth = _expandTypeWidth;
    expandTypeHeight = _expandTypeHeight;
    name = std::move(_name);
}

Window::Window(const WidgetInit& init)
    : Widget{init}
{}

auto Window::arrange(const layout::Rect& rect) -> bool
{
    return layer->arrange(rect);
}

auto Window::calculateSize() const -> WidgetSize
{
    auto widgetSize = layer->getWidgetSize();
    widgetSize.widthType = expandTypeWidth;
    widgetSize.heightType = expandTypeHeight;
    return widgetSize;
}

auto Window::calculateMinSize() const -> WidgetSize
{
    auto widgetMinSize = layer->getWidgetMinSize();
    widgetMinSize.widthType = expandTypeWidth;
    widgetMinSize.heightType = expandTypeHeight;
    return widgetMinSize;
}

auto Window::dropWindow() -> WindowDrop
{
    layout::Rect rect = getRect();
    return {name, rect, getTheme().dropImGuiStyleColors(context::ColorTheme::Window)};
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
