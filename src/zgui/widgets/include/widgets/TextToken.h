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
    ~TextToken() override = default;
    TextToken(const TextToken&) = delete;
    TextToken(TextToken&&) = delete;
    auto operator=(const TextToken&) -> TextToken& = delete;
    auto operator=(TextToken&&) -> TextToken& = delete;

    void setFontType(context::FontType fontType);
    auto hovered() const -> bool;
    void setNextFrameActive();
    auto clicked() -> bool;
    auto getPositionRect() const -> layout::Rect;
    auto getToken() const -> const annotation::Token&;

protected:
    auto calculateSize() const -> WidgetSize override;

private:
    using FontType = context::FontType;

    void renderShadow(float x, float y);
    void renderText(float x, float y) const;

    annotation::Token token;

    WidgetId shadowId{};
    FontType fontType{FontType::Gui};

    ColorSetId colorSetId;
    Color color;
    bool isHovered{false};
    bool isActive{false};
};

} // namespace widget
