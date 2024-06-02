#pragma once
#include <annotation/Tokenizer.h>
#include <widgets/Layer.h>
#include <widgets/TextToken.h>
#include <widgets/TextTokenSeq.h>

#include <memory>
#include <vector>

namespace gui {
class DisplayAnnotation
{
public:
    DisplayAnnotation(std::shared_ptr<widget::Layer> layer, std::vector<annotation::Alternative> alternatives);
    void draw();

private:
    constexpr static float s_border = 16.F;
    constexpr static float s_horizontalPadding = 64.F;
    constexpr static float s_padding = 24.F;
    constexpr static widget::BoxCfg boxCfg = {.padding = s_padding,
                                              .paddingHorizontal = s_horizontalPadding,
                                              .paddingVertical = s_padding,
                                              .border = s_border};

    void drawDialogue();
    void drawText();
    void setupDialogue();
    void setupText();

    widget::TextTokenSeq::Config ttqConfig;

    std::shared_ptr<widget::Layer> layer;
    std::vector<annotation::Alternative> alternatives;
};

} // namespace gui
