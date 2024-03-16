#pragma once
#include "Card.h"
#include "Token.h"
#include "ZH_Tokenizer.h"

#include <misc/Identifier.h>

#include <cstddef>
#include <memory>
#include <ranges>
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
    using Paragraph = std::vector<Token>;
    TokenText(std::shared_ptr<Card> card, std::vector<VocableId> vocableIds);
    void setupActiveVocables(const std::vector<VocableId>& activeVocableIds);
    [[nodiscard]] auto activeVocableIdsInOrder(const std::vector<VocableId>& activeVocableIds) -> std::vector<VocableId>;
    [[nodiscard]] auto getType() const -> TextType;
    [[nodiscard]] auto getParagraph() const -> const Paragraph&;
    [[nodiscard]] auto getDialogue() const -> const std::vector<Paragraph>&;

private:
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
