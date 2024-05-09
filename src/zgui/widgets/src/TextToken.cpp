#include <TextToken.h>
#include <annotation/Token.h>
#include <context/Fonts.h>
#include <detail/Widget.h>
#include <imgui.h>
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
    if (!token.getWord()) {
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
