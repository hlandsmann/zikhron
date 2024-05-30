#pragma once
#include "Card.h"
#include "Token.h"

#include <annotation/Ease.h>
#include <misc/Identifier.h>

#include <cstddef>
#include <map>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>

namespace annotation {
enum class TextType {
    dialogue,
    subtitle,
    text,
};

struct ActiveVocable
{
    VocableId vocableId;
    Ease ease;
    ColorId colorId;
};

class TokenText
{
public:
    using Paragraph = std::vector<Token>;
    TokenText(std::shared_ptr<Card> card, std::vector<VocableId> vocableIds);
    [[nodiscard]] auto setupActiveVocableIds(const std::map<VocableId, Ease>&) -> std::vector<ActiveVocable>;
    [[nodiscard]] auto getType() const -> TextType;
    [[nodiscard]] auto getParagraph() const -> const Paragraph&;
    [[nodiscard]] auto getDialogue() const -> const std::vector<Paragraph>&;

private:
    using tokenSubrange = std::ranges::subrange<std::vector<annotation::Token>::const_iterator>;
    void setupDialogueCard(const DialogueCard&);
    void setupTextCard(const TextCard&);
    [[nodiscard]] static auto tokenVector(tokenSubrange tokens) -> std::vector<Token>;
    [[nodiscard]] static auto findItAtThreshold(tokenSubrange tokens, std::size_t threshold)
            -> std::vector<annotation::Token>::const_iterator;

    TextType textType{TextType::dialogue};
    std::vector<Paragraph> paragraphSeq;
    std::shared_ptr<Card> card;
    std::vector<VocableId> vocableIds;
};

} // namespace annotation
