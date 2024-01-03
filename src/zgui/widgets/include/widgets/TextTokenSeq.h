#pragma once
#include "Box.h"
#include "TextToken.h"
#include "detail/Widget.h"

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

    auto arrange() -> bool override;
    void draw();

protected:
    auto calculateSize() const -> WidgetSize override;

private:
    constexpr static float border = 16;
    auto linesFit() const -> bool;
    static void addTextToken(Box& box, const annotation::Token& token);

    std::shared_ptr<Box> lines;
    std::shared_ptr<Box> scratchBox;
    Paragraph paragraph;
};

} // namespace widget
