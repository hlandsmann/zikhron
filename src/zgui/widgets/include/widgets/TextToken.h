#pragma once
#include <Widget.h>
#include <annotation/Token.h>
namespace widget {

class TextToken : public Widget<TextToken>
{
public:
    TextToken(const WidgetInit& init, annotation::Token token);

private:
    friend class Widget<TextToken>;
    auto calculateSize() const -> WidgetSize;
    annotation::Token token;
};

} // namespace widget
