#pragma once
#include <Widget.h>
#include <annotation/Token.h>
#include <context/Fonts.h>
namespace widget {

class TextToken : public Widget<TextToken>
{
public:
    TextToken(WidgetInit init, annotation::Token token);
    void setFontType(context::FontType fontType);
    void renderShadow();
    void clicked();

private:
    using FontType = context::FontType;

    friend class Widget<TextToken>;
    auto calculateSize() const -> WidgetSize;
    annotation::Token token;

    context::FontType fontType{FontType::Gui};
};

} // namespace widget
