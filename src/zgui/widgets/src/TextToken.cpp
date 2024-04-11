#include <TextToken.h>
#include <annotation/Token.h>
#include <context/Fonts.h>
#include <detail/Widget.h>
#include <imgui.h>
// #include <imgui_internal.h>
#include <misc/Identifier.h>

#include <string>
#include <utility>

namespace widget {

void TextToken::setup(annotation::Token _token)
{
    token = std::move(_token);
    shadowId = getWidgetIdGenerator()->getNextId();
}

TextToken::TextToken(WidgetInit _init)
    : Widget{std::move(_init)}
{
}

void TextToken::setFontType(context::FontType _fontType)
{
    fontType = _fontType;
}

void TextToken::renderShadow()
{
    constexpr int thickness = 2;
    const auto& rect = getRect();
    auto idDrop = dropWidgetId(shadowId);
    for (int xshift = 0; xshift <= thickness * 2; xshift++) {
        for (int yshift = 0; yshift <= thickness * 2; yshift++) {
            int xs = xshift - (thickness);
            int ys = yshift - (thickness);
            renderText(rect.x + static_cast<float>(xs),
                       rect.y + static_cast<float>(ys));
        }
    }
}

void TextToken::renderText(float x, float y) const
{
    ImGui::SetCursorPos({x, y});
    ImGui::Text("%s", token.string().data());
}

auto TextToken::testHovered() const -> bool
{
    const auto& rect = getRect();
    if (token.getDictionaryEntries().empty()) {
        return false;
    }
    renderText(rect.x, rect.y);
    return ImGui::IsItemHovered();
}

auto TextToken::clicked() -> bool
{
    const auto& rect = getRect();
    const auto& fontTheme = getTheme().getFont();
    auto fontDrop = fontTheme.dropFont(fontType);

    bool isHovered = testHovered();
    {
        auto colorDrop = isHovered ? fontTheme.dropFontColor(token.getColorId(), maxColorId)
                                   : fontTheme.dropShadowFontColor();
        renderShadow();
    }
    {
        auto idDrop = dropWidgetId();
        auto colorDrop = isHovered ? fontTheme.dropShadowFontColor()
                                   : fontTheme.dropFontColor(token.getColorId(), maxColorId);
        renderText(rect.x, rect.y);
        if (token.getDictionaryEntries().empty()) {
            return false;
        }
        return ImGui::IsItemClicked();
    }
}

auto TextToken::getPositionRect() const -> layout::Rect
{
    return getRect();
}

auto TextToken::getToken() const -> const annotation::Token&
{
    return token;
}

auto TextToken::calculateSize() const -> WidgetSize
{
    auto fontDrop = getTheme().getFont().dropFont(fontType);
    auto string = token.string();
    auto textSize = ImGui::CalcTextSize(string.cbegin().base(), string.cend().base());

    return {.width = textSize.x,
            .height = textSize.y};
}
} // namespace widget

namespace ImGui {
// void TextWithHoverColor(ImVec4 col, const std::string& fmt)
// {
//     // ImGuiContext& g = *GImGui;
//     ImGuiWindow* window = GetCurrentWindow();
//     if (window->SkipItems) {
//         return;
//     }
//
//     // Format text
//
//     // Layout
//     const ImVec2 text_pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
//     const ImVec2 text_size = CalcTextSize(fmt.cbegin().base(), fmt.cend().base());
//     ImRect bb(text_pos.x, text_pos.y, text_pos.x + text_size.x, text_pos.y + text_size.y);
//     ItemSize(text_size, 0.0F);
//     if (!ItemAdd(bb, 0)) {
//         return;
//     }
//
//     // Render
//     bool hovered = IsItemHovered();
//     if (hovered) {
//         PushStyleColor(ImGuiCol_Text, col);
//     }
//     RenderText(bb.Min, fmt.cbegin().base(), fmt.cend().base(), false);
//     if (hovered) {
//         PopStyleColor();
//     }
// }
} // namespace ImGui
