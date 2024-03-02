#pragma once
#include <annotation/TokenText.h>
#include <widgets/Grid.h>
#include <widgets/Layer.h>
#include <widgets/detail/Widget.h>

#include <memory>

namespace gui {

class DisplayText
{
    using Align = widget::layout::Align;

public:
    DisplayText(std::shared_ptr<widget::Layer> layer, std::unique_ptr<annotation::TokenText> tokenText);

    void draw();

private:
    void drawDialogue();
    void drawText();
    void setupDialogue();
    void setupText();

    std::shared_ptr<widget::Layer> layer;
    std::unique_ptr<annotation::TokenText> tokenText;
};

} // namespace gui
