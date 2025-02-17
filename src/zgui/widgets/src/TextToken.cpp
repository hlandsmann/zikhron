#include <TextToken.h>
#include <annotation/Token.h>
#include <context/Fonts.h>
#include <detail/Widget.h>
#include <imgui.h>
#include <misc/Identifier.h>

#include <string>
#include <utility>

namespace widget {

void TextToken::setup(const annotation::Token& _token)
{
    token = _token;
    const auto& colorSet = getTheme().getColorSet();
    color = colorSet.getColor(token.getColorId(), colorSetId);
    shadowId = getWidgetIdGenerator()->getNextId();
}

void TextToken::setup(const annotation::Token& _token, const Color& _color)
{
    setup(_token);
    color = _color;
}

void TextToken::setup(const annotation::Token& _token, ColorSetId _colorSetId)
{
    colorSetId = _colorSetId;
    setup(_token);
}

TextToken::TextToken(WidgetInit _init)
    : Widget{std::move(_init)}
    , colorSetId{getTheme().getColorSet().getColorSetId(0)}
{
}

void TextToken::setFontType(context::FontType _fontType)
{
    fontType = _fontType;
}

auto TextToken::hovered() const -> bool
{
    return isHovered;
}

void TextToken::setNextFrameActive()
{
    isActive = true;
}

void TextToken::renderShadow(float x, float y)
{
    constexpr int thickness = 2;
    auto idDrop = dropWidgetId(shadowId);
    for (int xshift = 0; xshift <= thickness * 2; xshift++) {
        for (int yshift = 0; yshift <= thickness * 2; yshift++) {
            int xs = xshift - (thickness);
            int ys = yshift - (thickness);
            renderText(x + static_cast<float>(xs),
                       y + static_cast<float>(ys));
        }
    }
}

void TextToken::renderText(float x, float y) const
{
    ImGui::SetCursorPos({x, y});
    ImGui::Text("%s", token.string().data());
}

auto TextToken::clicked() -> bool
{
    bool clicked{false};
    const auto& rect = getRect();
    const auto& fontTheme = getTheme().getFont();
    auto fontDrop = fontTheme.dropFont(fontType);

    const auto& colorSet = getTheme().getColorSet();
    const auto& shadowColor = colorSet.getColor(ColorId::shadowFontColor, {});

    {
        auto colorDrop = isActive ? context::FontColorDrop{color}
                                  : context::FontColorDrop{shadowColor};
        renderShadow(rect.x, rect.y);
    }
    {
        auto idDrop = dropWidgetId();
        auto colorDrop = isActive ? context::FontColorDrop{shadowColor}
                                  : context::FontColorDrop{color};
        renderText(rect.x, rect.y);

        isHovered = ImGui::IsItemHovered();
        if (token.getWord()) {
            isActive |= isHovered;
        }

        if (isActive) {
            clicked = ImGui::IsItemClicked();
        }
        isActive &= isHovered;
    }

    return clicked;
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
