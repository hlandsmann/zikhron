#pragma once
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <annotation/Token.h>
#include <context/ColorSet.h>
#include <context/Fonts.h>
#include <misc/Identifier.h>

namespace widget {

class TextToken : public Widget
{
    using Color = context::Color;
    using ColorSetId = context::ColorSetId;

    template<class T>
    friend class MetaBox;
    friend class Widget;
    void setup(const annotation::Token& token);
    void setup(const annotation::Token& token, const Color& color);
    void setup(const annotation::Token& token, ColorSetId colorSetId);

public:
    TextToken(WidgetInit init);

    void setFontType(context::FontType fontType);
    auto hovered() const -> bool;
    void setNextFrameActive();
    auto clicked() -> bool;
    auto getPositionRect() const -> layout::Rect;
    auto getToken() const -> const annotation::Token&;
    void resetWord();

protected:
    auto calculateSize() const -> WidgetSize override;

private:
    using FontType = context::FontType;

    void renderShadow();
    void renderText(float x, float y) const;
    auto testHovered() const -> bool;

    annotation::Token token;

    WidgetId shadowId{};
    FontType fontType{FontType::Gui};

    ColorSetId colorSetId;
    Color color;
    bool isHovered{false};
    bool isActive{false};
};

} // namespace widget
