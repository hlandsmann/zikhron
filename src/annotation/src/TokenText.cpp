#include <Card.h>
#include <Token.h>
#include <TokenText.h>
#include <ZH_Tokenizer.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <utils/StringU8.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <span>
#include <utility>
#include <vector>

namespace ranges = std::ranges;
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
}

auto TokenText::getType() const -> TextType
{
    return textType;
}

void TokenText::setupDialogueCard(const DialogueCard& dialogueCard)
{
    const ZH_Tokenizer& zh_tokenizer = dialogueCard.getTokenizer();
    auto fragmentBegin = zh_tokenizer.Tokens().begin();
    auto fragmentEnd = zh_tokenizer.Tokens().begin();
    std::size_t threshold = 0;
    auto textToParagraph = [&](const utl::StringU8& text) -> std::vector<Token> {
        threshold += utl::StringU8(text).length();
        fragmentEnd = findItAtThreshold({fragmentBegin, zh_tokenizer.Tokens().end()}, threshold);
        auto paragraph = tokenVector({fragmentBegin, fragmentEnd});
        fragmentBegin = fragmentEnd;
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

} // namespace annotation
