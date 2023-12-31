#include <ImageButton.h>
#include <Widget.h>
#include <context/Texture.h>
#include <context/Theme.h>
#include <context/WidgetState.h>
#include <context/imglog.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <magic_enum.hpp>
#include <string>
#include <utility>

namespace widget {

void ImageButton::setup(context::Image _image)
{
    label = magic_enum::enum_name(_image);
    image = _image;
}

ImageButton::ImageButton(WidgetInit init)
    : Widget{std::move(init)}
{}

auto ImageButton::clicked() -> bool
{
    const auto& btnRect = Rect();
    ImGui::SetCursorPos({btnRect.x, btnRect.y});
    auto tex = getTheme().getTexture().get(image);

    auto clicked = ImGui::ImageButton(label.c_str(), reinterpret_cast<void*>(tex.data), {tex.width, tex.height},
                                      ImVec2(0.0F, 0.0F),
                                      ImVec2(1.0F, 1.0F),
                                      backGroundColor,
                                      iconColor);
    context::WidgetState widgetState = context::getWidgetState(disabled, enabled);
    backGroundColor = getTheme().ColorButton(widgetState);
    iconColor = getTheme().ColorImage(widgetState);
    return clicked;
}

void ImageButton::setChecked(bool _checked)
{
    enabled = _checked;
}

void ImageButton::setSensitive(bool _sensitive)
{
    disabled = not _sensitive;
}

auto ImageButton::calculateSize() const -> WidgetSize
{
    auto tex = getTheme().getTexture().get(image);
    ImGui::ImageButton(label.c_str(), reinterpret_cast<void*>(tex.data), ImVec2(tex.width, tex.height));
    auto size = ImGui::GetItemRectSize();
    return {.widthType = PassiveOrientation() == layout::Orientation::horizontal ? layout::width_expand : layout::width_fixed,
            .heightType = PassiveOrientation() == layout::Orientation::vertical ? layout::height_expand : layout::height_fixed,
            .width = size.x,
            .height = size.y};
}
} // namespace widget
