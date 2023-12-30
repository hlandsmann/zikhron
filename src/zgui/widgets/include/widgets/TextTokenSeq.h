#pragma once
#include "Box.h"
#include "TextToken.h"
#include "Widget.h"

#include <annotation/TokenText.h>
#include <imgui.h>

#include <string>
#include <vector>
namespace widget {
class TextTokenSeq : public Widget<TextTokenSeq>
{
public:
    using Paragraph = annotation::TokenText::Paragraph;
    TextTokenSeq(WidgetInit init, Paragraph paragraph);
    auto arrange() -> bool override;
    void draw();

private:
    friend class Widget<TextTokenSeq>;
    auto calculateSize() const -> WidgetSize;

    Box lines;
    Paragraph paragraph;
};

} // namespace widget
