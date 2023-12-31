#include <Button.h>
#include <Widget.h>
#include <context/Theme.h>
#include <context/imglog.h>
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
    return {.widthType = PassiveOrientation() == layout::Orientation::horizontal ? layout::width_expand : layout::width_fixed,
            .heightType = PassiveOrientation() == layout::Orientation::vertical ? layout::height_expand : layout::height_fixed,
            .width = size.x + buttonPadding * 2,
            .height = size.y + buttonPadding * 2};
}

auto Button::clicked() const -> bool
{
    using ColorTheme = context::ColorTheme;
    auto styleColorDrop = getTheme().dropImGuiStyleColors(ColorTheme::ButtonDefault);
    const auto& btnRect = Rect();
    ImGui::SetCursorPos({btnRect.x, btnRect.y});

    return ImGui::Button(label.c_str(), {btnRect.width, btnRect.height});
}

void Button::setChecked(bool _checked)
{
    checked = _checked;
}

void Button::setSensitive(bool _sensitive)
{
    sensitive = not _sensitive;
}

} // namespace widget
