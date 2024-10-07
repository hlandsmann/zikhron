#include "DisplayText.h"

#include <VocableOverlay.h>
#include <annotation/TokenText.h>
#include <context/Fonts.h>
#include <misc/Language.h>
#include <utils/format.h>
#include <widgets/Grid.h>
#include <widgets/Layer.h>
#include <widgets/Overlay.h>
#include <widgets/TextToken.h>
#include <widgets/TextTokenSeq.h>

#include <memory>
#include <optional>
#include <utility>

namespace gui {

DisplayText::DisplayText(std::shared_ptr<widget::Layer> _layer,
                         std::shared_ptr<widget::Overlay> _overlay,
                         std::unique_ptr<annotation::TokenText> _tokenText,
                         Language _language)
    : layer{std::move(_layer)}
    , overlay{std::move(_overlay)}
    , tokenText{std::move(_tokenText)}
    , language{_language}
    , ttqConfig{.fontType = context::getFontType(context::FontSize::big, language)}
    , colorSetId{layer->getTheme().getColorSet().getColorSetId(tokenText->getVocableCount())}
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

auto DisplayText::draw() -> bool
{
    std::optional<std::shared_ptr<widget::TextToken>> optTextToken;
    bool configured = false;

    using annotation::TextType;
    switch (tokenText->getType()) {
    case TextType::dialogue:
        optTextToken = drawDialogue();
        break;
    case TextType::subtitle:
    case TextType::text:
        optTextToken = drawText();
        break;
    }
    if (vocableOverlay) {
        vocableOverlay->draw();
        configured = vocableOverlay->configured();
        if (vocableOverlay->shouldClose()) {
            vocableOverlay.reset();
        }
    }
    if (optTextToken.has_value()) {
        vocableOverlay = std::make_unique<VocableOverlay>(overlay, optTextToken.value(), language);
    }
    return configured;
}

auto DisplayText::drawDialogue() -> std::optional<std::shared_ptr<widget::TextToken>>
{
    std::optional<std::shared_ptr<widget::TextToken>> result;
    auto& grid = layer->getWidget<widget::Grid>(textWidgetId);
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
    auto& ttq = layer->getWidget<widget::TextTokenSeq>(textWidgetId);
    return ttq.draw();
}

void DisplayText::setupDialogue()
{
    int index = 0;
    auto grid = layer->add<widget::Grid>(Align::start, 2, widget::Grid::Priorities{0.3F, 0.7F});
    textWidgetId = grid->getWidgetId();
    grid->setBorder(16.F);
    grid->setHorizontalPadding(64.F);
    grid->setVerticalPadding(24.F);
    for (const auto& dialogue : tokenText->getDialogue()) {
        auto ttq = grid->add<widget::TextTokenSeq>(Align::start, dialogue, ttqConfig, colorSetId);
        ttq->setName(fmt::format("ttq_{}", index));
        index++;
    }
}

void DisplayText::setupText()
{
    ttqConfig.border = s_border;
    auto ttq = layer->add<widget::TextTokenSeq>(Align::start, tokenText->getParagraph(), ttqConfig, colorSetId);
    textWidgetId = ttq->getWidgetId();
}

} // namespace gui
