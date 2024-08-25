#include "SteppedSlider.h"

#include <imgui.h>
#include <utils/format.h>

#include <cstddef>
#include <string>

namespace widget {
void SteppedSlider::setup() {}

SteppedSlider::SteppedSlider(const WidgetInit& init)
    : Widget{init}
{}

auto SteppedSlider::slide(std::size_t max, std::size_t pos) -> std::size_t
{
    auto widgetIdDrop = dropWidgetId();

    const auto& rect = getRect();
    ImGui::SetCursorPos({rect.x, rect.y});
    ImGui::SetNextItemWidth(rect.width);
    std::string label = labelString(max, pos);
    int value = static_cast<int>(pos);
    ImGui::SliderInt("##", &value, 0, static_cast<int>(max), label.c_str());

    return static_cast<std::size_t>(value);
}

auto SteppedSlider::labelString(std::size_t max, std::size_t pos) -> std::string
{
    return fmt::format("{} / {}", pos, max);
}

auto SteppedSlider::calculateSize() const -> WidgetSize
{
    auto widgetIdDrop = dropWidgetId();
    int tempValue{};
    ImGui::SliderInt("##", &tempValue, 0, 1);
    auto size = ImGui::GetItemRectSize();

    return {.width = size.x,
            .height = size.y};
}

auto SteppedSlider::getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize
{
    auto widgetIdDrop = dropWidgetId();

    ImGui::SetNextItemWidth(rect.width);

    int tempValue{};
    ImGui::SliderInt("##", &tempValue, 0, 1);
    auto size = ImGui::GetItemRectSize();

    return {.width = size.x,
            .height = size.y};
}
} // namespace widget
