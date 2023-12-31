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
class TextTokenSeq : public Widget
{
    using Paragraph = annotation::TokenText::Paragraph;
    friend class Box;
    void setup(Paragraph paragraph);

public:
    TextTokenSeq(WidgetInit init);
    auto arrange() -> bool override;
    void draw();

    protected:
    auto calculateSize() const -> WidgetSize override;
private:
    auto linesFit() const -> bool;

    std::shared_ptr<Box> lines;
    Paragraph paragraph;
};

} // namespace widget
