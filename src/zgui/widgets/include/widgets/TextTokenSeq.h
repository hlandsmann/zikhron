#pragma once
#include "Box.h"
#include "TextToken.h"
#include "Widget.h"

#include <annotation/TokenText.h>
#include <imgui.h>

#include <memory>
#include <string>
#include <vector>
namespace widget {
class TextTokenSeq : public Widget<TextTokenSeq>
{
    using Paragraph = annotation::TokenText::Paragraph;
    friend class Box;
    void setup(Paragraph paragraph);

public:
    TextTokenSeq(WidgetInit init);
    auto arrange() -> bool override;
    void draw();

private:
    friend class Widget<TextTokenSeq>;
    auto calculateSize() const -> WidgetSize;

    std::shared_ptr<Box> lines;
    Paragraph paragraph;
};

} // namespace widget
