#include "Window.h"

#include <Box.h>
#include <Layer.h>
#include <context/Theme.h>
#include <context/imglog.h>
#include <detail/Widget.h>
#include <imgui.h>

#include <memory>
#include <string>
#include <utility>

namespace widget {
void Window::setup(ExpandType _expandTypeWidth,
                   ExpandType _expandTypeHeight,
                   const std::string& _name)
{
    layer = std::make_shared<widget::Layer>(widget::WidgetInit{
            .theme = getThemePtr(),
            .widgetIdGenerator = getWidgetIdGenerator(),
            .rect = std::make_shared<layout::Rect>(),
            .horizontalAlign = widget::layout::Align::start,
            .verticalAlign = widget::layout::Align::start,
            .expandTypeWidth = _expandTypeWidth,
            .expandTypeHeight = _expandTypeHeight,
            .parent = shared_from_this(),
    });
    setExpandType(_expandTypeWidth, _expandTypeHeight);
    setName(_name);
}

Window::Window(const WidgetInit& init)
    : Widget{init}
{}

auto Window::arrange(const layout::Rect& rect) -> bool
{
    setRect(rect);

    auto layerRect = rect;
    layerRect.x = 0;
    layerRect.y = 0;
    return layer->arrange(layerRect);
}

auto Window::getLayer() const -> Layer&
{
    return *layer;
}

auto Window::calculateSize() const -> WidgetSize
{
    auto widgetSize = layer->getWidgetSize();
    return widgetSize;
}

auto Window::calculateMinSize() const -> WidgetSize
{
    auto widgetMinSize = layer->getWidgetMinSize();
    return widgetMinSize;
}

auto Window::getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize
{
    auto getWidgetSizeFromRect = layer->getWidgetSizeFromRect(rect);
    return getWidgetSizeFromRect;
}

auto Window::dropWindow() -> WindowDrop
{
    start();
    layout::Rect rect = getRect();
    // imglog::log("dropwin: {}, arrange, x: {}, y: {}, w: {}, h: {}", getName(), rect.x, rect.y, rect.width, rect.height);
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
