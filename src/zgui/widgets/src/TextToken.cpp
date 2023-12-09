#include <TextToken.h>
#include <Widget.h>
#include <annotation/Token.h>
#include <context/Fonts.h>
#include <imgui.h>

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
    const auto& btnRect = Rect();
    ImGui::SetCursorPos({btnRect.x, btnRect.y});
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
