#include "Child.h"

#include <Layer.h>
#include <Window.h>
#include <context/Theme.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

namespace widget {

void Child::setup(float _width, const std::string& _name)
{
    layer = std::make_shared<widget::Layer>(widget::WidgetInit{
            .theme = getThemePtr(),
            .widgetIdGenerator = getWidgetIdGenerator(),
            .rect = std::make_shared<layout::Rect>(),
            .horizontalAlign = widget::layout::Align::start,
            .verticalAlign = widget::layout::Align::start,
            .expandTypeWidth = ExpandType::width_fixed,
            .expandTypeHeight = ExpandType::height_fixed,
            .parent = shared_from_this(),
    });
    setExpandType(ExpandType::width_fixed, ExpandType::height_fixed);
    setName(_name);
    width = _width;
}

void Child::setup(const std::string& _name)
{
    setup(0.F, _name);
}

Child::Child(const WidgetInit& init)
    : Widget{init}
{}

auto Child::arrange(const layout::Rect& rect) -> bool
{
    setRect(rect);
    auto layerRect = rect;
    layerRect.x = 0;
    layerRect.y = 0;
    return layer->arrange(layerRect);
}

auto Child::calculateSize() const -> WidgetSize
{
    auto widgetSize = layer->getWidgetSize();
    if (width != 0.F) {
        widgetSize.width = width;
    }
    return widgetSize;
}

auto Child::calculateMinSize() const -> WidgetSize
{
    auto widgetMinSize = layer->getWidgetMinSize();
    if (width != 0.F) {
        widgetMinSize.width = width;
    }
    return widgetMinSize;
}

auto Child::dropChild() -> ChildDrop
{
    start();
    layout::Rect rect = getRect();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    auto offset = getOffset();
    float thickness = 1;
    draw_list->AddRect({rect.x + offset.x - thickness,
                        rect.y + offset.y - thickness},
                       {rect.x + offset.x + thickness + rect.width,
                        rect.y + offset.y + thickness + rect.height},
                       ImGui::ColorConvertFloat4ToU32(getTheme().ColorBorder()));
    return {getWidgetIdName(), rect, getTheme().dropImGuiStyleColors(context::ColorTheme::Child)};
}

ChildDrop::ChildDrop(const std::string& name, const widget::layout::Rect& rect,
                     context::StyleColorsDrop _styleColorsDrop)
    : styleColorsDrop{std::move(_styleColorsDrop)}
{
    // ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 10.F);
    // ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.59F, 0.59F, 0.59F, 1.F));
    ImGui::SetCursorPos({rect.x, rect.y});
    ImGui::BeginChild(name.c_str(), {rect.width, rect.height});
    incPopCount();
}

void ChildDrop::pop()
{
    ImGui::EndChild();
    // ImGui::PopStyleColor();
    // ImGui::PopStyleVar();
}
} // namespace widget
