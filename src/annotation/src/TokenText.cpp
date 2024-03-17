#include "TokenText.h"

#include <Card.h>
#include <Token.h>
#include <ZH_Tokenizer.h>
#include <annotation/Ease.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <utils/StringU8.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <ranges>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ranges = std::ranges;
namespace views = std::ranges::views;

namespace annotation {

TokenText::TokenText(std::shared_ptr<Card> _card, std::vector<VocableId> _vocableIds)
    : card{std::move(_card)}
    , vocableIds{std::move(_vocableIds)}
{
    if (const auto* dlgCard = dynamic_cast<const DialogueCard*>(card.get())) {
        textType = TextType::dialogue;
        setupDialogueCard(*dlgCard);
    }
    if (const auto* textCard = dynamic_cast<const TextCard*>(card.get())) {
        textType = TextType::text;
        setupTextCard(*textCard);
    }
    setVocableIdsForTokens();
}

auto TokenText::setupActiveVocableIds(const std::map<VocableId, Ease>& vocId_ease) -> std::vector<std::pair<VocableId, Ease>>
{
    std::vector<std::pair<VocableId, Ease>> orderedVocId_ease{};
    auto tokens = views::join(paragraphSeq);
    ranges::copy(vocId_ease, std::back_inserter(orderedVocId_ease));
    ranges::sort(orderedVocId_ease, std::less{}, [&](const auto& pairVocId_ease) {
        const auto& [vocId, _] = pairVocId_ease;
        auto it = ranges::find_if(tokens, [vocId](const auto& token) -> bool {
            return token.getVocableId() == vocId;
        });
        return ranges::distance(tokens.begin(), it);
    });
    for (auto& token : tokens) {
        auto it = ranges::find(orderedVocId_ease, token.getVocableId(), &decltype(orderedVocId_ease)::value_type::first);
        if (it != orderedVocId_ease.end()) {
            auto colorId = std::distance(orderedVocId_ease.begin(), it) + 1;
            token.setColorId(static_cast<ColorId>(colorId));
        }
    }

    return orderedVocId_ease;
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

void TokenText::setupDialogueCard(const DialogueCard& dialogueCard)
{
    const ZH_Tokenizer& zh_tokenizer = dialogueCard.getTokenizer();
    auto fragmentBegin = zh_tokenizer.Tokens().begin();
    auto fragmentEnd = zh_tokenizer.Tokens().begin();
    auto textToParagraph = [&](const utl::StringU8& text) -> std::vector<Token> {
        auto threshold = utl::StringU8(text).length();
        fragmentEnd = findItAtThreshold({fragmentBegin, zh_tokenizer.Tokens().end()}, threshold);
        auto paragraph = tokenVector({fragmentBegin, fragmentEnd});
        fragmentBegin = fragmentEnd + 1; // +1 a '~'-character is generated at the end of each subtext (speaker, dialogue)
        return paragraph;
    };
    for (const auto& dialogue : dialogueCard.dialogue) {
        paragraphSeq.push_back(textToParagraph(dialogue.speaker));
        paragraphSeq.push_back(textToParagraph(dialogue.text));
    }
}

void TokenText::setupTextCard(const TextCard& textCard)
{
    const ZH_Tokenizer& zh_tokenizer = textCard.getTokenizer();
    paragraphSeq.push_back(tokenVector({zh_tokenizer.Tokens()}));
}

auto TokenText::tokenVector(tokenSubrange tokens) -> std::vector<Token>
{
    auto tokensOut = std::vector<Token>(tokens.size());
    ranges::transform(tokens, tokensOut.begin(),
                      [](const ZH_Tokenizer::Token& token) -> Token {
                          return {token.text, token.dicItemVec};
                      });
    return tokensOut;
}

auto TokenText::findItAtThreshold(tokenSubrange tokens, std::size_t threshold)
        -> std::vector<ZH_Tokenizer::Token>::const_iterator
{
    std::size_t size{};
    for (auto it = tokens.begin(); it != tokens.end(); it++) {
        size += it->text.length();
        if (size == threshold) {
            return it + 1;
        }
    }
    spdlog::error("Could not find token iterator at threshold {}", threshold);
    return tokens.end();
}

void TokenText::setVocableIdsForTokens()
{
    auto tokens = views::join(paragraphSeq);
    for (auto& token : tokens) {
        const auto& dictionaryEntries = token.getDictionaryEntries();
        auto itDicEntry = ranges::find_if(dictionaryEntries, [this](const auto& entry) -> bool {
            return ranges::find(vocableIds, entry.id) != vocableIds.end();
        });
        if (itDicEntry != dictionaryEntries.end()) {
            token.setVocableId(itDicEntry->id);
        };
    }
}

} // namespace annotation
