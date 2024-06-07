#include "DisplayAnnotation.h"

#include <TokenizationOverlay.h>
#include <annotation/Token.h>
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
#include <cassert>
#include <generator>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace gui {

DisplayAnnotation::DisplayAnnotation(std::shared_ptr<widget::Layer> _layer,
                                     std::shared_ptr<widget::Overlay> _overlay,
                                     std::vector<annotation::Alternative> _alternatives,
                                     std::unique_ptr<annotation::TokenText> _tokenText)
    : layer{std::move(_layer)}
    , overlay{std::move(_overlay)}
    , alternatives{std::move(_alternatives)}
    , tokenText{std::move(_tokenText)}
{
    tokenText->setupAnnotation(alternatives);
    ranges::for_each(tokenText->traverseToken(), &annotation::Token::resetWord);
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
    if (tokenizationOverlay) {
        tokenizationOverlay->draw();
        if (tokenizationOverlay->shouldClose()) {
            tokenizationOverlay.reset();
        }
    }
    auto optAlternateToken = alternativeClicked();
    if (optAlternateToken.has_value()) {
        auto& [token, alternative] = optAlternateToken.value();
        tokenizationOverlay = std::make_unique<TokenizationOverlay>(overlay, token, alternative);
    }
}

void DisplayAnnotation::drawDialogue()
{
    auto& grid = layer->getWidget<widget::Grid>(textWidgetId);
    grid.start();
    while (!grid.isLast()) {
        auto& ttq = grid.next<widget::TextTokenSeq>();
        auto optResult = ttq.draw();
    }
}

void DisplayAnnotation::drawText()
{
    auto& ttq = layer->getWidget<widget::TextTokenSeq>(textWidgetId);
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
        index++;
    }
}

void DisplayAnnotation::setupText()
{
    ttqConfig.border = s_border;
    auto ttq = layer->add<widget::TextTokenSeq>(Align::start, tokenText->getParagraph(), ttqConfig, colorSetId);
    textWidgetId = ttq->getWidgetId();
}

auto DisplayAnnotation::traverseToken() -> std::generator<const std::shared_ptr<widget::TextToken>&>
{
    using annotation::TextType;
    switch (tokenText->getType()) {
    case TextType::dialogue: {
        auto& grid = layer->getWidget<widget::Grid>(textWidgetId);
        grid.start();
        while (!grid.isLast()) {
            auto& ttq = grid.next<widget::TextTokenSeq>();
            for (const auto& token : ttq.traverseToken()) {
                co_yield token;
            }
        }
    } break;
    case TextType::subtitle:
    case TextType::text: {
        auto& ttq = layer->getWidget<widget::TextTokenSeq>(textWidgetId);
        for (const auto& token : ttq.traverseToken()) {
            co_yield token;
        }
        break;
    }
    }
}

auto DisplayAnnotation::alternativeClicked() -> std::optional<TokenAlternative>
{
    std::vector<std::shared_ptr<widget::TextToken>> alternateTextToken;
    auto alternativeIt = alternatives.cbegin();
    auto currentIt = alternativeIt->current.cbegin();
    for (const auto& textToken : traverseToken()) {
        const auto& token = textToken->getToken();
        if (*currentIt != token.getValue() && *currentIt == std::string{"~"}) {
            std::advance(alternativeIt, 1);
            currentIt = alternativeIt->current.cbegin();
        }
        assert(*currentIt == token.getValue());
        if (!alternativeIt->candidates.empty()) {
            alternateTextToken.push_back(textToken);
        }
        std::advance(currentIt, 1);
        if (currentIt == alternativeIt->current.end()) {
            if (!alternativeIt->candidates.empty()) {
                if (ranges::any_of(alternateTextToken, &widget::TextToken::hovered)) {
                    ranges::for_each(alternateTextToken, &widget::TextToken::setNextFrameActive);
                }
                if (ranges::any_of(alternateTextToken, &widget::TextToken::clicked)) {
                    return {{alternateTextToken.front(), alternativeIt}};
                }
                alternateTextToken.clear();
            }
            std::advance(alternativeIt, 1);
            currentIt = alternativeIt->current.begin();
        }
    }
    return {};
}

} // namespace gui
