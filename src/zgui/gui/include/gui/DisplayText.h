#pragma once
#include <annotation/TokenText.h>
#include <context/Fonts.h>
#include <widgets/Grid.h>
#include <widgets/Layer.h>
#include <widgets/TextTokenSeq.h>
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

    widget::TextTokenSeq::Config ttqConfig;

    std::shared_ptr<widget::Layer> layer;
    std::unique_ptr<annotation::TokenText> tokenText;
};

} // namespace gui
