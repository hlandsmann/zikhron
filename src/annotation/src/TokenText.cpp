#include "TokenText.h"

#include <annotation/Ease.h>
#include <annotation/Token.h>
#include <annotation/Tokenizer.h>
#include <database/Card.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <utils/StringU8.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <generator>
#include <iterator>
#include <map>
#include <memory>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace ranges = std::ranges;
namespace views = std::ranges::views;

namespace annotation {

TokenText::TokenText(std::shared_ptr<Card> _card)
    : card{std::move(_card)}

{
    if (const auto* dlgCard = dynamic_cast<const DialogueCard*>(card.get())) {
        textType = TextType::dialogue;
        setupDialogueCard(*dlgCard);
    }
    if (const auto* textCard = dynamic_cast<const TextCard*>(card.get())) {
        textType = TextType::text;
        setupTextCard(*textCard);
    }
    if (const auto* subtitleCard = dynamic_cast<const SubtitleCard*>(card.get())) {
        textType = TextType::subtitle;
        setupSubtitleCard(*subtitleCard);
    }
}

auto TokenText::setupActiveVocableIds(const std::map<VocableId, Ease>& vocId_ease) -> std::vector<ActiveVocable>
{
    std::vector<ActiveVocable> activeVocables{};
    auto tokens = views::join(paragraphSeq);
    ranges::transform(vocId_ease, std::back_inserter(activeVocables), [](const auto& pairVocId_ease) -> ActiveVocable {
        return {.vocableId = pairVocId_ease.first, .ease = pairVocId_ease.second, .colorId = {}};
    });
    ranges::sort(activeVocables, std::less{}, [&](const auto& activeVocable) {
        auto it = ranges::find_if(tokens, [&](const auto& token) -> bool {
            return token.getVocableId() == activeVocable.vocableId;
        });
        return ranges::distance(tokens.begin(), it);
    });

    std::size_t colorId = 0;
    for (auto& activeVocable : activeVocables) {
        activeVocable.colorId = static_cast<ColorId>(colorId);
        colorId++;
    }
    std::size_t nextColorId = 0;

    for (auto& token : tokens) {
        auto it = ranges::find(activeVocables, token.getVocableId(), &ActiveVocable::vocableId);
        if (it != activeVocables.end()) {
            if (it->colorId >= nextColorId) {
                token.setColorId(it->colorId);
                nextColorId = it->colorId + 1;
            }
        }
    }
    vocableCount = vocId_ease.size();
    return activeVocables;
}

void TokenText::setupAnnotation(const std::vector<annotation::Alternative>& alternatives)
{
    auto tokens = views::join(paragraphSeq);
    auto alternativeIt = alternatives.cbegin();
    auto currentIt = alternativeIt->current.cbegin();
    unsigned alternatingColor = 0;
    bool altColor = false;
    for (auto& token : tokens) {
        if (*currentIt != token.getValue() && *currentIt == std::string{"~"}) {
            std::advance(alternativeIt, 1);
            currentIt = alternativeIt->current.cbegin();
        }
        assert(*currentIt == token.getValue());

        if (!alternativeIt->candidates.empty()) {
            token.setColorId(static_cast<ColorId>(alternatingColor * 2 + (altColor ? 1 : 0)));
            altColor = !altColor;
        }
        std::advance(currentIt, 1);
        if (currentIt == alternativeIt->current.end()) {
            if (!alternativeIt->candidates.empty()) {
                alternatingColor++;
                altColor = false;
            }
            std::advance(alternativeIt, 1);
            currentIt = alternativeIt->current.begin();
        }
    }
}

auto TokenText::getType() const -> TextType
{
    return textType;
}

auto TokenText::getParagraph() const -> const Paragraph&
{
    if (textType != TextType::subtitle && textType != TextType::text) {
        throw std::runtime_error("TokenText is neither of type subtitle nor text");
    }
    return paragraphSeq.front();
}

auto TokenText::getDialogue() const -> const std::vector<Paragraph>&
{
    if (textType != TextType::dialogue) {
        throw std::runtime_error("TokenText is not of type dialogue");
    }
    return paragraphSeq;
}

auto TokenText::getVocableCount() const -> std::size_t
{
    return vocableCount;
}

auto TokenText::traverseToken() -> std::generator<Token&>
{
    for (auto& paragraph : paragraphSeq) {
        for (auto& token : paragraph) {
            co_yield token;
        }
    }
    co_return;
}

void TokenText::setupDialogueCard(const DialogueCard& dialogueCard)
{
    const auto& tokens = dialogueCard.getTokens();
    auto fragmentBegin = tokens.begin();
    auto fragmentEnd = tokens.begin();
    auto textToParagraph = [&](const utl::StringU8& text) -> std::vector<Token> {
        auto threshold = utl::StringU8(text).length();
        fragmentEnd = findItAtThreshold({fragmentBegin, tokens.end()}, threshold);
        auto paragraph = tokenVector({fragmentBegin, fragmentEnd});
        fragmentBegin = fragmentEnd + 1; // +1 a '~'-character is generated at the end of each subtext (speaker, dialogue)
        return paragraph;
    };
    for (const auto& dialogue : dialogueCard.getDialogue()) {
        paragraphSeq.push_back(textToParagraph(dialogue.speaker));
        paragraphSeq.push_back(textToParagraph(dialogue.text));
    }
}

void TokenText::setupTextCard(const TextCard& textCard)
{
    paragraphSeq.push_back(textCard.getTokens());
}

void TokenText::setupSubtitleCard(const SubtitleCard& subtitleCard)
{
    paragraphSeq.push_back(subtitleCard.getTokens());
}

auto TokenText::tokenVector(tokenSubrange tokens) -> std::vector<Token>
{
    auto tokensOut = std::vector<Token>(tokens.size());
    ranges::copy(tokens, tokensOut.begin());
    return tokensOut;
}

auto TokenText::findItAtThreshold(tokenSubrange tokens, std::size_t threshold)
        -> std::vector<annotation::Token>::const_iterator
{
    std::size_t size{};
    for (auto it = tokens.begin(); it != tokens.end(); it++) {
        size += it->getValue().length();
        if (size == threshold) {
            return it + 1;
        }
    }
    spdlog::error("Could not find token iterator at threshold {}", threshold);
    return tokens.end();
}

} // namespace annotation
