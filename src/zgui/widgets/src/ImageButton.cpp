#include <ImageButton.h>
#include <context/Texture.h>
#include <context/Theme.h>
#include <context/WidgetState.h>
#include <context/imglog.h>
#include <detail/Widget.h>
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
    auto widgetIdDrop = dropWidgetId();
    const auto& btnRect = getRect();
    ImGui::SetCursorPos({btnRect.x, btnRect.y});
    auto tex = getTheme().getTexture().get(image);

    auto clicked = ImGui::ImageButton(label.c_str(), reinterpret_cast<void*>(tex.data), {tex.width, tex.height},
                                      ImVec2(0.0F, 0.0F),
                                      ImVec2(1.0F, 1.0F),
                                      backGroundColor,
                                      iconColor);
    context::WidgetState widgetState = context::getWidgetState(sensitive, checked);
    backGroundColor = getTheme().ColorButton(widgetState);
    iconColor = getTheme().ColorImage(widgetState);
    return clicked;
}

void ImageButton::setChecked(bool _checked)
{
    checked = _checked;
}

void ImageButton::setSensitive(bool _sensitive)
{
    sensitive = _sensitive;
}

auto ImageButton::isChecked() const -> bool
{
    return checked;
}

auto ImageButton::isSensitive() const -> bool
{
    return sensitive;
}

auto ImageButton::calculateSize() const -> WidgetSize
{
    auto tex = getTheme().getTexture().get(image);
    ImGui::ImageButton(label.c_str(), reinterpret_cast<void*>(tex.data), ImVec2(tex.width, tex.height));
    auto size = ImGui::GetItemRectSize();
    return {.width = size.x,
            .height = size.y};
}
} // namespace widget
