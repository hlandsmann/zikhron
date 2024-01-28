#include "Window.h"

#include <Box.h>
#include <Layer.h>
#include <context/Theme.h>
#include <context/imglog.h>
#include <detail/Widget.h>
#include <imgui.h>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>

namespace widget {
void Window::setup(ExpandType _expandTypeWidth,
                   ExpandType _expandTypeHeight,
                   std::string _name)
{
    layer = std::make_shared<widget::Layer>(widget::WidgetInit{
            .theme = getThemePtr(),
            .widgetIdGenerator = getWidgetIdGenerator(),
            .name = std::move(_name),
            .rect = std::make_shared<layout::Rect>(),
            .orientation = widget::layout::Orientation::horizontal,
            .horizontalAlign = widget::layout::Align::start,
            .verticalAlign = widget::layout::Align::start,
            .parent = std::weak_ptr<widget::Widget>{}

    });
    expandTypeWidth = _expandTypeWidth;
    expandTypeHeight = _expandTypeHeight;
}

Window::Window(const WidgetInit& init)
    : Widget{init}
{}

auto Window::arrange(const layout::Rect& rect) -> bool
{
    setRect(rect);
    return layer->arrange(rect);
}

auto Window::getWidgetSize(const layout::Rect& rect) -> WidgetSize
{
    imglog::log("win: {}, arrange, x: {}, y: {}, w: {}, h: {}", getName(), rect.x, rect.y, rect.width, rect.height);
    auto widgetSize = dynamic_cast<const Widget&>(*layer).getWidgetSize();
    if (expandTypeWidth == ExpandType::expand) {
        widgetSize.width = rect.width;
    }
    if (expandTypeHeight == ExpandType::expand) {
        widgetSize.height = rect.height;
    }
    return widgetSize;
}

auto Window::calculateSize() const -> WidgetSize
{
    auto widgetSize = dynamic_cast<const Widget&>(*layer).getWidgetSize();
    return widgetSize;
}

auto Window::calculateMinSize() const -> WidgetSize
{
    auto widgetMinSize = layer->getWidgetMinSize();
    return widgetMinSize;
}

auto Window::dropWindow() -> WindowDrop
{
    layout::Rect rect = getRect();
    imglog::log("dropwin: {}, arrange, x: {}, y: {}, w: {}, h: {}", getName(), rect.x, rect.y, rect.width, rect.height);
    return {getName(), rect, getTheme().dropImGuiStyleColors(context::ColorTheme::Window)};
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
