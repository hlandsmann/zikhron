#pragma once
#include <annotation/TokenText.h>
#include <context/Fonts.h>
#include <widgets/Grid.h>
#include <widgets/Layer.h>
#include <widgets/TextToken.h>
#include <widgets/TextTokenSeq.h>
#include <widgets/detail/Widget.h>

#include <memory>
#include <optional>

namespace gui {

class DisplayText
{
    using Align = widget::layout::Align;

public:
    DisplayText(std::shared_ptr<widget::Layer> layer, std::unique_ptr<annotation::TokenText> tokenText);

    auto draw() -> std::optional<std::shared_ptr<widget::TextToken>>;

private:
    auto drawDialogue() -> std::optional<std::shared_ptr<widget::TextToken>>;
    auto drawText() -> std::optional<std::shared_ptr<widget::TextToken>>;
    void setupDialogue();
    void setupText();

    widget::TextTokenSeq::Config ttqConfig;

    std::shared_ptr<widget::Layer> layer;
    std::unique_ptr<annotation::TokenText> tokenText;
};

} // namespace gui
