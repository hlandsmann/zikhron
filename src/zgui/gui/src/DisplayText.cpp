#include "DisplayText.h"

#include <annotation/TokenText.h>
#include <context/Fonts.h>
#include <utils/spdlog.h>
#include <widgets/Layer.h>
#include <widgets/TextToken.h>
#include <widgets/TextTokenSeq.h>

#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

namespace gui {

DisplayText::DisplayText(std::shared_ptr<widget::Layer> _layer, std::unique_ptr<annotation::TokenText> _tokenText)
    : layer{std::move(_layer)}
    , tokenText{std::move(_tokenText)}
{
    ttqConfig.fontType = context::FontType::chineseBig;
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

auto DisplayText::draw() -> std::optional<std::shared_ptr<widget::TextToken>>
{
    layer->start();
    if (layer->isLast()) {
        throw std::runtime_error("Empty DisplayText is invalid!");
    }
    using annotation::TextType;
    switch (tokenText->getType()) {
    case TextType::dialogue:
        return drawDialogue();
        break;
    case TextType::subtitle:
    case TextType::text:
        return drawText();
        break;
    }
    return std::nullopt;
}

auto DisplayText::drawDialogue() -> std::optional<std::shared_ptr<widget::TextToken>>
{
    std::optional<std::shared_ptr<widget::TextToken>> result;
    auto& grid = layer->next<widget::Grid>();
    grid.start();
    while (!grid.isLast()) {
        auto optResult = grid.next<widget::TextTokenSeq>().draw();
        if (optResult.has_value()) {
            result = std::move(optResult);
        }
    }
    return result;
}

auto DisplayText::drawText() -> std::optional<std::shared_ptr<widget::TextToken>>
{
    return layer->next<widget::TextTokenSeq>().draw();
}

void DisplayText::setupDialogue()
{
    int index = 0;
    auto grid = layer->add<widget::Grid>(Align::start, 2, widget::Grid::Priorities{0.3F, 0.7F});
    grid->setBorder(16.F);
    grid->setHorizontalPadding(64.F);
    grid->setVerticalPadding(24.F);
    for (const auto& dialogue : tokenText->getDialogue()) {
        auto ttq = grid->add<widget::TextTokenSeq>(Align::start, dialogue, ttqConfig);
        ttq->setName(fmt::format("ttq_{}", index));
        index++;
    }
}

void DisplayText::setupText()
{
    ttqConfig.border = s_border;
    layer->add<widget::TextTokenSeq>(Align::start, tokenText->getParagraph(), ttqConfig);
}

} // namespace gui
