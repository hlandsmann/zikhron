#pragma once
#include "Card.h"
#include "Token.h"
#include "ZH_Tokenizer.h"

#include <misc/Identifier.h>

#include <cstddef>
#include <ranges>
#include <memory>
#include <span>
#include <vector>

namespace annotation {
enum class TextType {
    dialogue,
    subtitle,
    text,
};
class TokenText
{
public:
    TokenText(std::shared_ptr<Card> card, std::vector<VocableId> vocableIds);
    [[nodiscard]] auto getType() const -> TextType;

private:
    using Paragraph = std::vector<Token>;
    using tokenSubrange = std::ranges::subrange<std::vector<ZH_Tokenizer::Token>::const_iterator>;
    void setupDialogueCard(const DialogueCard&);
    void setupTextCard(const TextCard&);
    [[nodiscard]] static auto tokenVector(tokenSubrange tokens) -> std::vector<Token>;
    [[nodiscard]] static auto findItAtThreshold(tokenSubrange tokens, std::size_t threshold)
            -> std::vector<ZH_Tokenizer::Token>::const_iterator;

    TextType textType{TextType::dialogue};
    std::vector<Paragraph> paragraphSeq;
    std::shared_ptr<Card> card;
    std::vector<VocableId> vocableIds;
};

} // namespace annotation
