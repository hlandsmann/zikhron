#include "TokenizationOverlay.h"

#include <annotation/Token.h>
#include <annotation/Tokenizer.h>
#include <misc/Identifier.h>
#include <utils/StringU8.h>
#include <widgets/Box.h>
#include <widgets/Overlay.h>
#include <widgets/TextToken.h>
#include <widgets/TextTokenSeq.h>

#include <algorithm>
#include <iterator>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace gui {

TokenizationOverlay::TokenizationOverlay(std::shared_ptr<widget::Overlay> _overlay,
                                         std::shared_ptr<widget::TextToken> _textToken,
                                         std::vector<annotation::Alternative>::const_iterator _alternative)
    : overlay{std::move(_overlay)}
    , textToken{std::move(_textToken)}
    , alternative{*_alternative}
    , current{alternative.current}
    , colorId{textToken.lock()->getToken().getColorId()}
{
    using namespace widget::layout;
    overlay->setMaxWidth(std::numeric_limits<float>::max());
    overlay->clear();
    overlay->setFirstDrop();
    auto box = overlay->add<widget::Box>(Align::start, boxCfg, widget::Orientation::vertical);
    box->setName("overlayBox");
    setupBox();
}

void TokenizationOverlay::draw()
{
    auto ltoken = textToken.lock();
    if (!ltoken) {
        return;
    }
    auto rect = ltoken->getPositionRect();

    auto drop = overlay->dropOverlay(rect.x - s_border, rect.y - s_border);
    overlay->start();
    auto& box = overlay->next<widget::Box>();
    box.start();

    box.next<widget::Box>(); // currentBox
    drawCurrent();

    drawAlternatives(box);
}

auto TokenizationOverlay::shouldClose() const -> bool
{
    return overlay->shouldClose();
}

auto TokenizationOverlay::configured() const -> bool
{
    return wasConfigured;
}

void TokenizationOverlay::setupBox()
{
    overlay->start();
    auto& box = overlay->next<widget::Box>();
    currentBox = box.add<widget::Box>(Align::start, widget::Orientation::vertical);

    setupCurrent();
    setupAlternatives(box);
}

void TokenizationOverlay::setupCurrent()
{
    currentBox->clear();
    addAlternateStrU8Vector(*currentBox, current);
}

void TokenizationOverlay::drawCurrent()
{
    currentBox->start();
    currentBox->next<widget::TextTokenSeq>().draw();
}

void TokenizationOverlay::setupAlternatives(widget::Box& box)
{
    for (const auto& candidate : alternative.candidates) {
        addAlternateStrU8Vector(box, candidate);
    }
}

void TokenizationOverlay::drawAlternatives(widget::Box& box)
{
    auto candidateIt = alternative.candidates.begin();
    bool hovered = false;
    while (!box.isLast()) {
        auto& ttq = box.next<widget::TextTokenSeq>();
        ttq.draw();

        if (ranges::any_of(ttq.traverseToken(), &widget::TextToken::hovered)) {
            ranges::for_each(ttq.traverseToken(), &widget::TextToken::setNextFrameActive);
            current = *candidateIt;
            setupCurrent();
            hovered = true;
        }
        std::advance(candidateIt, 1);
    }
    if (!hovered && current != alternative.current) {
        current = alternative.current;
        setupCurrent();
    }
}

void TokenizationOverlay::addAlternateStrU8Vector(widget::Box& box, std::vector<utl::StringU8> altStrU8Vec)
{
    std::vector<annotation::Token> currentTokens;
    ranges::transform(altStrU8Vec, std::back_inserter(currentTokens), &annotation::Token::fromStringU8);
    alternateColorsForTokens(currentTokens);
    box.add<widget::TextTokenSeq>(Align::start, currentTokens, ttqConfig, colorSetId);
}

void TokenizationOverlay::alternateColorsForTokens(std::vector<annotation::Token>& tokens) const
{
    bool alternateColor{false};
    for (auto& token : tokens) {
        token.setColorId(alternateColor ? static_cast<ColorId>(colorId + 1) : colorId);
        alternateColor = !alternateColor;
    }
}
} // namespace gui
