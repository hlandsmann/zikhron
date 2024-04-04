#include <MediaSlider.h>
#include <detail/Widget.h>
#include <imgui.h>

#include <format>
#include <optional>
#include <string>

namespace widget {
void MediaSlider::setup() {}

MediaSlider::MediaSlider(const WidgetInit& init)
    : Widget{init}
{}

auto MediaSlider::slide(float _value) -> std::optional<float>
{
    auto widgetIdDrop = dropWidgetId();
    float value = _value;
    const auto& rect = getRect();
    ImGui::SetCursorPos({rect.x, rect.y});
    ImGui::SetNextItemWidth(rect.width);
    std::string timeStamp = std::format("h: {:.2f}", value);
    ImGui::SliderFloat("##", &value, 0.F, 1.F, timeStamp.c_str());
    if (value != _value) {
        return {value};
    }
    return {};
}

// auto MediaSlider::calculateSize() const -> WidgetSize
// {
// }

auto MediaSlider::calculateSize() const -> WidgetSize
{
    auto widgetIdDrop = dropWidgetId();
    float tempValue{};
    ImGui::SliderFloat("##", &tempValue, 0.F, 1.F);
    auto size = ImGui::GetItemRectSize();

    return {.width = size.x,
            .height = size.y};
}

auto MediaSlider::getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize
{
    auto widgetIdDrop = dropWidgetId();

    ImGui::SetNextItemWidth(rect.width);

    float tempValue{};
    ImGui::SliderFloat("##", &tempValue, 0.F, 1.F);
    auto size = ImGui::GetItemRectSize();

    return {.width = size.x,
            .height = size.y};
}

} // namespace widget
