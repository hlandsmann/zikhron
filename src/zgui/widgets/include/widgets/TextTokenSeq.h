#pragma once
#include "Box.h"
#include "TextToken.h"
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <annotation/Token.h>
#include <annotation/TokenText.h>
#include <imgui.h>

#include <memory>
#include <string>
#include <vector>

namespace widget {
class TextTokenSeq : public Widget
{
    using Paragraph = annotation::TokenText::Paragraph;
    using Align = widget::layout::Align;

public:
    void setup(Paragraph paragraph);
    TextTokenSeq(WidgetInit init);

    void draw();

private:
    auto calculateSize() const -> WidgetSize override;
    auto calculateMinSize() const -> WidgetSize override;

    auto arrange(const layout::Rect& /* rect */) -> bool override;
    [[nodiscard]] auto getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize override;

    constexpr static float border = 16;
    auto arrangeLines(Box& lines, const layout::Rect& rect) -> bool;
    auto linesFit(const layout::Rect& rect) const -> bool;
    static void addTextToken(Box& box, const annotation::Token& token);

    layout::Rect lastRect;
    std::shared_ptr<Box> lineBox;
    std::shared_ptr<Box> scratchBox;
    Paragraph paragraph;
};

} // namespace widget
