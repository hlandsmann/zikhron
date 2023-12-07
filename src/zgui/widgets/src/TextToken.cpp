#include <TextToken.h>
#include <Widget.h>
#include <annotation/Token.h>
#include <imgui.h>

#include <utility>
namespace widget {

TextToken::TextToken(const WidgetInit& _init, annotation::Token _token)
    : Widget<TextToken>{std::move(_init)}
    , token{std::move(_token)}
{
}

auto TextToken::calculateSize() const -> WidgetSize
{
    auto string = token.string();
    auto textSize = ImGui::CalcTextSize(string.cbegin().base(), string.cend().base());

    return {.widthType = layout::width_fixed,
            .heightType = layout::height_fixed,
            .width = textSize.x,
            .height = textSize.y};
}
} // namespace widget
