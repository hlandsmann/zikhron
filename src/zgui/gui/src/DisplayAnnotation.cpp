#include "DisplayAnnotation.h"

#include <annotation/TokenText.h>
#include <annotation/Tokenizer.h>
#include <context/Fonts.h>
#include <utils/spdlog.h>
#include <widgets/Grid.h>
#include <widgets/Layer.h>
#include <widgets/Overlay.h>
#include <widgets/TextToken.h>
#include <widgets/TextTokenSeq.h>

#include <algorithm>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace gui {

DisplayAnnotation::DisplayAnnotation(std::shared_ptr<widget::Layer> _layer,
                                     std::vector<annotation::Alternative> _alternatives,
                                     std::unique_ptr<annotation::TokenText> _tokenText)
    : layer{std::move(_layer)}
    , alternatives{std::move(_alternatives)}
    , tokenText{std::move(_tokenText)}
{
    tokenText->setupAnnotation(alternatives);
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

void DisplayAnnotation::draw()
{
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

void DisplayAnnotation::drawDialogue()
{
    auto& grid = layer->getWidget<widget::Grid>(textWidgetId);
    grid.start();
    while (!grid.isLast()) {
        auto& ttq = grid.next<widget::TextTokenSeq>();
        ranges::for_each(ttq.traverseToken(), &widget::TextToken::resetWord);
        auto optResult = ttq.draw();
        // if (optResult.has_value()) {
        //     result = std::move(optResult);
        // }
    }
}

void DisplayAnnotation::drawText()
{
    auto& ttq = layer->getWidget<widget::TextTokenSeq>(textWidgetId);
    ranges::for_each(ttq.traverseToken(), &widget::TextToken::resetWord);
    ttq.draw();
}

void DisplayAnnotation::setupDialogue()
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
        // for(const auto& token: ttq->traverseToken()){
        for (const auto& token : ttq->traverseToken()) {
            spdlog::info("reset: {}", token->getToken().getValue());
            token->resetWord();
        }
        spdlog::info("reset");

        index++;
    }
}

void DisplayAnnotation::setupText()
{
    ttqConfig.border = s_border;
    auto ttq = layer->add<widget::TextTokenSeq>(Align::start, tokenText->getParagraph(), ttqConfig, colorSetId);
    ranges::for_each(ttq->traverseToken(), &widget::TextToken::resetWord);
    textWidgetId = ttq->getWidgetId();
    for (const auto& token : ttq->traverseToken()) {
        spdlog::info("reset: {}", token->getToken().getValue());
        token->resetWord();
    }
}

// auto Fonts::getFontColorAlternative(std::size_t indexAlt, bool alt) const -> const ImVec4&
// {
//     auto index = indexAlt % (fontColorsAlternatives.size() / 2);
//     index = index * 2 + (alt ? 1 : 0);
//     return fontColorsAlternatives.at(index);
// }
} // namespace gui
