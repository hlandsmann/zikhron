#pragma once
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <annotation/Token.h>
#include <context/Fonts.h>
#include <misc/Identifier.h>

namespace widget {

class TextToken : public Widget
{
    template<class T>
    friend class MetaBox;
    friend class Widget;
    void setup(annotation::Token token);

public:
    TextToken(WidgetInit init);

    void setFontType(context::FontType fontType);
    auto clicked() -> bool;
    auto getPositionRect() const -> layout::Rect;
    auto getToken() const -> const annotation::Token&;

protected:
    auto calculateSize() const -> WidgetSize override;

private:
    using FontType = context::FontType;

    void renderShadow();
    void renderText(float x, float y) const;
    auto testHovered() const -> bool;

    annotation::Token token;

    WidgetId shadowId{};
    context::FontType fontType{FontType::Gui};
    static constexpr ColorId maxColorId{};
};

} // namespace widget
