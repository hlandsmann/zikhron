#include <MediaSlider.h>
#include <detail/Widget.h>
#include <imgui.h>
#include <utils/format.h>

#include <chrono>
#include <string>

namespace widget {
void MediaSlider::setup() {}

MediaSlider::MediaSlider(const WidgetInit& init)
    : Widget{init}
{}

auto MediaSlider::slide(double start, double end, double pos) -> double
{
    auto widgetIdDrop = dropWidgetId();

    const auto& rect = getRect();
    ImGui::SetCursorPos({rect.x, rect.y});
    ImGui::SetNextItemWidth(rect.width);
    std::string timeStamp = timeString(start, pos); // std::format("h: {:.2f}", value);

    float value = sliderValueFromPos(start, end, pos);
    float oldValue = value;

    if (useKeyboard) {
        value = valueFromKeyboard(value);
    }

    ImGui::SliderFloat("##", &value, 0.F, 1.F, timeStamp.c_str());
    if (value == lastValue) {
        return pos;
    }
    lastValue = value;
    if (value != oldValue) {
        return posFromSliderValue(start, end, value);
    }
    return pos;
}

void MediaSlider::setUseKeyboard(bool _useKeyboard)
{
    useKeyboard = _useKeyboard;
}

auto MediaSlider::valueFromKeyboard(float value)  -> float
{
    if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_1)) {
        return 1.F / 10.F * 0.F;
    }
    if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_2)) {
        return 1.F / 10.F * 1.F;
    }
    if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_3)) {
        return 1.F / 10.F * 2.F;
    }
    if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_4)) {
        return 1.F / 10.F * 3.F;
    }
    if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_5)) {
        return 1.F / 10.F * 4.F;
    }
    if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_6)) {
        return 1.F / 10.F * 5.F;
    }
    if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_7)) {
        return 1.F / 10.F * 6.F;
    }
    if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_8)) {
        return 1.F / 10.F * 7.F;
    }
    if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_9)) {
        return 1.F / 10.F * 8.F;
    }
    if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_0)) {
        return 1.F / 10.F * 9.F;
    }

    return value;
}

auto MediaSlider::sliderValueFromPos(double start, double end, double pos) -> float
{
    double timeSpan = end - start;
    if (timeSpan == 0.) {
        return 0.F;
    }
    return static_cast<float>((pos - start) / timeSpan);
}

auto MediaSlider::posFromSliderValue(double start, double end, float value) -> double
{
    return (static_cast<double>(value) * (end - start)) + start;
}

auto MediaSlider::timeString(double start, double pos) -> std::string
{
    namespace chrono = std::chrono;
    auto relativePos = chrono::duration<double>(pos - start);
    auto hours = chrono::duration_cast<chrono::hours>(relativePos);
    relativePos -= hours;
    auto minutes = chrono::duration_cast<chrono::minutes>(relativePos);
    relativePos -= minutes;
    auto seconds = chrono::duration_cast<chrono::seconds>(relativePos);
    relativePos -= seconds;
    auto milliseconds = chrono::duration_cast<chrono::milliseconds>(relativePos);
    if (hours.count() == 0) {
        return fmt::format("{:02d}:{:02d}.{:02d}", minutes.count(),
                           seconds.count(), milliseconds.count() / 10);
    }
    return fmt::format("{:02d}:{:02d}:{:02d}", hours.count(), minutes.count(), seconds.count());
}

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
