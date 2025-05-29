#include "ScrollArea.h"

#include <Layer.h>
#include <Window.h>
#include <context/Theme.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>

namespace widget {

void ScrollArea::setup(const std::string& _name)
{
    layer = std::make_shared<widget::Layer>(widget::WidgetInit{
            .theme = getThemePtr(),
            .widgetIdGenerator = getWidgetIdGenerator(),
            .rect = std::make_shared<layout::Rect>(),
            .horizontalAlign = widget::layout::Align::start,
            .verticalAlign = widget::layout::Align::start,
            .expandTypeWidth = ExpandType::width_adapt,
            .expandTypeHeight = ExpandType::height_adapt,
            .parent = shared_from_this(),
    });
    setName(_name);
}

ScrollArea::ScrollArea(const WidgetInit& init)
    : Widget{init}
{}

auto ScrollArea::arrange(const layout::Rect& rect) -> bool
{
    setRect(rect);
    layout::Rect layerRect = {
            .x = 0,
            .y = 0,
            .width = rect.width,
            .height = std::numeric_limits<float>::max(),
    };
    if (scrollbarIsActive) {
        layerRect.width -= scrollbarWidth;
    }

    return layer->arrange(layerRect);
}

auto ScrollArea::calculateSize() const -> WidgetSize
{
    auto widgetSize = layer->getWidgetSize();
    if (!scrollbarIsActive) {
        widgetSize.width += scrollbarWidth;
    }
    return widgetSize;
}

auto ScrollArea::calculateMinSize() const -> WidgetSize
{
    auto widgetMinSize = layer->getWidgetMinSize();
    if (!scrollbarIsActive) {
        widgetMinSize.width += scrollbarWidth;
    }
    return widgetMinSize;
}

auto ScrollArea::getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize
{
    if (layer->numberOfWidgets() == 0) {
        return {};
    }
    layout::Rect preliminaryRect = {
            .x = 0,
            .y = 0,
            .width = rect.width,
            .height = std::numeric_limits<float>::max()};

    if (rect.width - scrollbarWidth >= widgetSizeCache.width) {
        preliminaryRect.width -= scrollbarWidth;
    }
    auto areaSize = layer->getWidgetSizeFromRect(preliminaryRect);
    scrollbarIsActive = areaSize.height > rect.height;
    if (scrollbarIsActive) {
        areaSize.width += scrollbarWidth;
    }
    widgetSizeCache = areaSize;
    return areaSize;
}

auto ScrollArea::dropScrollArea() -> ScrollAreaDrop
{
    start();
    layout::Rect rect = getRect();
    return {getWidgetIdName(), rect, getTheme().dropImGuiStyleColors(context::ColorTheme::Child)};
}

ScrollAreaDrop::ScrollAreaDrop(const std::string& name, const widget::layout::Rect& rect,
                               context::StyleColorsDrop _styleColorsDrop)
    : styleColorsDrop{std::move(_styleColorsDrop)}
{
    ImGui::SetCursorPos({rect.x, rect.y});
    ImGui::BeginChild(name.c_str(), {rect.width, rect.height});
    incPopCount();
}

void ScrollAreaDrop::pop()
{
    ImGui::EndChild();
}
} // namespace widget
