#pragma once
#include <annotation/Ease.h>
#include <annotation/Token.h>
#include <annotation/Tokenizer.h>
#include <database/Card.h>
#include <misc/Identifier.h>

#include <cstddef>
#include <generator>
#include <map>
#include <memory>
#include <ranges>
#include <vector>

namespace annotation {
enum class TextType {
    dialogue,
    subtitle,
    text,
};

struct ColoredVocable
{
    VocableId vocableId;
    ColorId colorId;
};

class TokenText
{
    using Card = database::Card;
    using DialogueCard = database::DialogueCard;
    using TextCard = database::TextCard;
    using SubtitleCard = database::SubtitleCard;

public:
    using Paragraph = std::vector<Token>;
    TokenText(std::shared_ptr<database::Card> card);
    [[nodiscard]] auto setupActiveVocableIds(const std::vector<VocableId>&) -> std::vector<ColoredVocable>;
    void setupAnnotation(const std::vector<annotation::Alternative>& alternatives);
    [[nodiscard]] auto getType() const -> TextType;
    [[nodiscard]] auto getParagraph() const -> const Paragraph&;
    [[nodiscard]] auto getDialogue() const -> const std::vector<Paragraph>&;
    [[nodiscard]] auto getVocableCount() const -> std::size_t;
    auto traverseToken() -> std::generator<Token&>;

private:
    using tokenSubrange = std::ranges::subrange<std::vector<Token>::const_iterator>;
    void setupDialogueCard(const DialogueCard&);
    void setupTextCard(const TextCard&);
    void setupSubtitleCard(const SubtitleCard&);
    [[nodiscard]] static auto tokenVector(tokenSubrange tokens) -> std::vector<Token>;
    [[nodiscard]] static auto findItAtThreshold(tokenSubrange tokens, std::size_t threshold)
            -> std::vector<annotation::Token>::const_iterator;

    TextType textType{TextType::dialogue};
    std::vector<Paragraph> paragraphSeq;
    std::shared_ptr<database::Card> card;
    std::size_t vocableCount{};
};

} // namespace annotation
