#pragma once
#include "Widget.h"

#include <annotation/Token.h>
#include <context/Fonts.h>
namespace widget {

class TextToken : public Widget
{
public:
    void setup(annotation::Token token);
    TextToken(WidgetInit init);

    void setFontType(context::FontType fontType);
    void renderShadow();
    void clicked();

protected:
    auto calculateSize() const -> WidgetSize override;

private:
    using FontType = context::FontType;

    annotation::Token token;

    context::FontType fontType{FontType::Gui};
};

} // namespace widget
