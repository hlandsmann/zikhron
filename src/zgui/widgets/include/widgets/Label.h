#pragma once
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <context/Fonts.h>

#include <string>

namespace widget {

class Label : public Widget
{
    template<class T>
    friend class MetaBox;
    friend class Widget;
    void setup(const std::string& text, context::FontType fontType);

public:
    Label(WidgetInit init);
    ~Label() override = default;

    Label(const Label&) = delete;
    Label(Label&&) = delete;
    auto operator=(const Label&) -> Label& = delete;
    auto operator=(Label&&) -> Label& = delete;

    void draw();

protected:
    auto calculateSize() const -> WidgetSize override;
    [[nodiscard]] auto getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize override;

private:
    context::FontType fontType{context::FontType::Gui};
    std::string text;
    WidgetSize sizeFromRect;
    WidgetSize textSize;
};
} // namespace widget
