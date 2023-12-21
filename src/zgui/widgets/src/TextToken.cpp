#include <TextToken.h>
#include <Widget.h>
#include <annotation/Token.h>
#include <context/Fonts.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <string>
#include <utility>
namespace widget {

TextToken::TextToken(WidgetInit _init, annotation::Token _token)
    : Widget<TextToken>{std::move(_init)}
    , token{std::move(_token)}
{
}

void TextToken::setFontType(context::FontType _fontType)
{
    fontType = _fontType;
}

void TextToken::renderShadow()
{
    auto fontDrop = getTheme().getFont().dropFont(fontType);
}

void TextToken::clicked()
{
    auto fontDrop = getTheme().getFont().dropFont(fontType);
    auto colorDrop = getTheme().getFont().dropDefaultFontColor();
    const auto& btnRect = Rect();
    ImGui::SetCursorPos({btnRect.x, btnRect.y});
    ImGui::Text("%s", token.string().data());
}

auto TextToken::calculateSize() const -> WidgetSize
{
    auto fontDrop = getTheme().getFont().dropFont(fontType);
    auto string = token.string();
    auto textSize = ImGui::CalcTextSize(string.cbegin().base(), string.cend().base());

    return {.widthType = layout::width_fixed,
            .heightType = layout::height_fixed,
            .width = textSize.x,
            .height = textSize.y};
}
} // namespace widget

namespace ImGui {
void TextWithHoverColor(ImVec4 col, const std::string& fmt)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return;
    }

    // Format text

    // Layout
    const ImVec2 text_pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
    const ImVec2 text_size = CalcTextSize(fmt.cbegin().base(), fmt.cend().base());
    ImRect bb(text_pos.x, text_pos.y, text_pos.x + text_size.x, text_pos.y + text_size.y);
    ItemSize(text_size, 0.0F);
    if (!ItemAdd(bb, 0)) {
        return;
    }

    // Render
    bool hovered = IsItemHovered();
    if (hovered) {
        PushStyleColor(ImGuiCol_Text, col);
    }
    RenderText(bb.Min, fmt.cbegin().base(), fmt.cend().base(), false);
    if (hovered) {
        PopStyleColor();
    }
}
} // namespace ImGui
