#include "Label.h"

#include <context/Fonts.h>
#include <imgui.h>

#include <algorithm>
#include <string>
#include <utility>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include <imgui_internal.h>
#pragma GCC diagnostic pop

namespace widget {
void Label::setup(const std::string& _text, context::FontType _fontType)
{
    text = _text;
    fontType = _fontType;
}

Label::Label(WidgetInit init)
    : Widget{std::move(init)}
{
}

void Label::draw()
{
    auto rect = getRect();
    auto fontDrop = getTheme().getFont().dropFont(fontType);
    if (textSize.width <= sizeFromRect.width) {
        auto widgetIdDrop = dropWidgetId();
        ImGui::SetCursorPos({rect.x, rect.y});
        ImGui::Text("%s", text.c_str());
        return;
    }

    ImGui::SetCursorPos({0, 0});
    auto offset = ImGui::GetCursorScreenPos();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    rect.x += offset.x;
    rect.y += offset.y;
    constexpr float ellipsis_max_x_magic = 8;
    ImGui::RenderTextEllipsis(draw_list, /* draw_list */
                              {rect.x,   /* pos_min */
                               rect.y},
                              {rect.x + rect.width, /* pos_max */
                               rect.y + rect.height},
                              rect.width + rect.x, /* clip_max_x */
                              rect.width + rect.x  /* ellipsis_max_x */
                                      + ellipsis_max_x_magic,
                              text.c_str(), nullptr, nullptr);
}

void Label::setText(const std::string& _text)
{
    text = _text;
    resetWidgetSize();
}

auto Label::calculateSize() const -> WidgetSize
{
    return sizeFromRect;
}

auto Label::getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize
{
    auto fontDrop = getTheme().getFont().dropFont(fontType);
    auto [textWidth, textHeight] = ImGui::CalcTextSize(text.cbegin().base(), text.cend().base());
    textSize = {.width = textWidth, .height = textHeight};
    sizeFromRect.width = std::min(rect.width, textWidth);
    sizeFromRect.height = textHeight;
    resetWidgetSize();
    return sizeFromRect;
}

} // namespace widget
