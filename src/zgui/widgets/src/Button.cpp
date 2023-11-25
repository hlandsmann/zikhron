#include <Button.h>
#include <Widget.h>
#include <context/Theme.h>
#include <context/imglog.h>
#include <imgui.h>

#include <string>
#include <utility>
namespace widget {

Button::Button(WidgetInit init,
               std::string _label)
    : Widget<Button>{std::move(init)}
    , label{std::move(_label)}
{}

auto Button::calculateSize() const -> WidgetSize
{
    ImGui::Button(label.c_str());
    auto size = ImGui::GetItemRectSize();
    return {.widthType = Orientation() == layout::Orientation::horizontal ? layout::width_expand : layout::width_fixed,
            .heightType = Orientation() == layout::Orientation::vertical ? layout::height_expand : layout::height_fixed,
            .width = size.x + buttonPadding * 2,
            .height = size.y + buttonPadding * 2};
}

auto Button::clicked() const -> bool
{
    auto styleColorDrop = getTheme().dropImGuiStyleColors();
    const auto& btnRect = Rect();
    ImGui::SetCursorPos({btnRect.x, btnRect.y});

    return ImGui::Button(label.c_str(), {btnRect.width, btnRect.height});
}

} // namespace widget
