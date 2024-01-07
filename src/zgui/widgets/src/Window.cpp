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
    layerRect = std::make_shared<layout::Rect>();
    // box = std::make_shared<Box>(getThemePtr(), PassiveOrientation(), std::weak_ptr{shared_from_this()});
    layer = std::make_shared<Layer>(WidgetInit{.theme = getThemePtr(),
                                           .widgetIdGenerator = getWidgetIdGenerator(),
                                           .rect = layerRect,
                                           .orientation = PassiveOrientation(),
                                           .horizontalAlign = Align::start,
                                           .verticalAlign = Align::start,
                                           .parent = std::weak_ptr{shared_from_this()}});
    
    layer->add<Box>(Align::start);
    expandTypeWidth = _expandTypeWidth;
    expandTypeHeight = _expandTypeHeight;
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
    *layerRect = Rect();
    layerRect->x = 0;
    layerRect->y = 0;
    return layer->arrange();
}

auto Window::getBox() -> Box&
{
    return getBox(0);
}

auto Window::getBox(std::size_t index) -> Box&
{
    return layer->getLayer<Box>(index);
}

auto Window::calculateSize() const -> WidgetSize
{
    auto size = layer->getWidgetSize();
    return {.widthType = expandTypeWidth,
            .heightType = expandTypeHeight,
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
