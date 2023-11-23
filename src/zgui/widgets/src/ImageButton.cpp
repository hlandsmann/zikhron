#include <ImageButton.h>
#include <Widget.h>
#include <context/Texture.h>
#include <context/Theme.h>
#include <imgui.h>

#include <string>
#include <utility>

namespace widget {

ImageButton::ImageButton(WidgetInit init,
                         std::string _label,
                         context::Image _image)
    : Widget<ImageButton>{std::move(init)}
    , label{std::move(_label)}
    , image{_image} {}

auto ImageButton::clicked() -> bool
{
    const auto& btnRect = Rect();
    ImGui::SetCursorPos({btnRect.x, btnRect.y});
    auto tex = getTheme().getTexture().get(image);

    auto clicked = ImGui::ImageButton(label.c_str(), reinterpret_cast<void*>(tex.data), {btnRect.width, btnRect.height},
                                      ImVec2(0.0F, 0.0F),
                                      ImVec2(1.0F, 1.0F),
                                      backGroundColor,
                                      iconColor);
    context::WidgetState widgetState = context::getWidgetState(disabled, enabled);
    backGroundColor = getTheme().ColorButton(widgetState);
    iconColor = getTheme().ColorImage(widgetState);
    return clicked;
}

auto ImageButton::calculateSize() const -> WidgetSize
{
    auto tex = getTheme().getTexture().get(image);
    ImGui::ImageButton(label.c_str(), reinterpret_cast<void*>(tex.data), ImVec2(tex.width, tex.height));
    auto size = ImGui::GetItemRectSize();
    return {.widthType = Orientation() == layout::Orientation::horizontal ? layout::width_expand : layout::width_fixed,
            .heightType = Orientation() == layout::Orientation::vertical ? layout::height_expand : layout::height_fixed,
            .width = size.x,
            .height = size.y};
}
} // namespace widget
