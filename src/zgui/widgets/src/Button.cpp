#include <Button.h>
#include <context/Theme.h>
#include <context/imglog.h>
#include <detail/Widget.h>
#include <imgui.h>

#include <string>
#include <utility>

namespace widget {
void Button::setup(std::string _label)
{
    label = std::move(_label);
}

Button::Button(WidgetInit init)
    : Widget{std::move(init)}
{}

auto Button::calculateSize() const -> WidgetSize
{
    ImGui::Button(label.c_str());
    auto size = ImGui::GetItemRectSize();
    return {.width = size.x + buttonPadding * 2,
            .height = size.y + buttonPadding * 2};
}

auto Button::clicked() const -> bool
{
    auto widgetIdDrop = dropWidgetId();

    using ColorTheme = context::ColorTheme;
    auto styleColorDrop = getTheme().dropImGuiStyleColors(checked
                                                                  ? ColorTheme::ButtonChecked
                                                                  : ColorTheme::ButtonDefault);
    const auto& btnRect = getRect();
    ImGui::SetCursorPos({btnRect.x, btnRect.y});
    // imglog::log("Button: x: {}, y: {}, w: {}, h: {}", btnRect.x, btnRect.x, btnRect.width, btnRect.height);

    return ImGui::Button(label.c_str(), {btnRect.width, btnRect.height});
}

void Button::setChecked(bool _checked)
{
    checked = _checked;
}

void Button::setSensitive(bool _sensitive)
{
    sensitive = _sensitive;
}
auto Button::isChecked() const -> bool
{
    return checked;
}

auto Button::isSensitive() const -> bool
{
    return sensitive;
}

} // namespace widget
