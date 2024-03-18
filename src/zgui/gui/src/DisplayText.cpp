#include "DisplayText.h"

#include <annotation/TokenText.h>
#include <context/Fonts.h>
#include <widgets/Layer.h>
#include <widgets/TextTokenSeq.h>

#include <memory>
#include <stdexcept>
#include <utility>

namespace gui {

DisplayText::DisplayText(std::shared_ptr<widget::Layer> _layer, std::unique_ptr<annotation::TokenText> _tokenText)
    : layer{std::move(_layer)}
    , tokenText{std::move(_tokenText)}
{
    using annotation::TextType;
    switch (tokenText->getType()) {
    case TextType::dialogue:
        setupDialogue();
        break;
    case TextType::subtitle:
    case TextType::text:
        setupText();
        break;
    }
}

void DisplayText::draw()
{
    layer->start();
    if (layer->isLast()) {
        throw std::runtime_error("Empty DisplayText is invalid!");
    }
    using annotation::TextType;
    switch (tokenText->getType()) {
    case TextType::dialogue:
        drawDialogue();
        break;
    case TextType::subtitle:
    case TextType::text:
        drawText();
        break;
    }
}

void DisplayText::drawDialogue()
{
    auto& grid = layer->next<widget::Grid>();
    grid.start();
    while (!grid.isLast()) {
        grid.next<widget::TextTokenSeq>().draw();
    }
}

void DisplayText::drawText()
{
    layer->next<widget::TextTokenSeq>().draw();
}

void DisplayText::setupDialogue()
{
    auto grid = layer->add<widget::Grid>(Align::start, 2, widget::Grid::Priorities{0.3F, 0.7F});
    for (const auto& dialogue : tokenText->getDialogue()) {
        grid->add<widget::TextTokenSeq>(Align::start, dialogue, fontType);
    }
}

void DisplayText::setupText()
{
    layer->add<widget::TextTokenSeq>(Align::start, tokenText->getParagraph(), fontType);
}

} // namespace gui
