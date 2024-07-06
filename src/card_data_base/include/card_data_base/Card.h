#pragma once
#include <annotation/JieBa.h>
#include <annotation/Token.h>
#include <annotation/Tokenizer.h>
#include "CbdFwd.h"
#include <dictionary/WordDB.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Identifier.h>
#include <unicode/unistr.h>
#include <utils/StringU8.h>

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <sys/types.h>

namespace annotation {

struct CardInit
{
    std::string packName;
    PackId packId;
    std::size_t indexInPack;
    std::shared_ptr<WordDB> wordDB;
    std::shared_ptr<Tokenizer> tokenizer;
};

class Card
{
public:
    using CharacterSequence = std::vector<utl::CharU8>;
    static auto deserializeCard(std::string_view content, const CardInit& cardInit) -> CardPtr;

    Card(const CardInit& cardInit);
    Card(const Card&) = default;
    Card(Card&&) = default;
    virtual ~Card() = default;
    auto operator=(const Card&) = delete;
    auto operator=(Card&&) = delete;

    void setCardId(CardId);
    [[nodiscard]] auto getCardId() const -> CardId;
    [[nodiscard]] auto getTokens() const -> const std::vector<Token>&;
    [[nodiscard]] auto getWordDB() const -> std::shared_ptr<WordDB>;
    [[nodiscard]] auto getAlternatives() const -> std::vector<Alternative>;
    [[nodiscard]] virtual auto serialize() const -> std::string = 0;
    [[nodiscard]] auto getPackId() const -> PackId;
    [[nodiscard]] auto getPackName() const -> std::string;
    [[nodiscard]] auto getIndexInPack() const -> std::size_t;
    void setTokenizationChoices(const TokenizationChoiceVec& tokenizationChoices);

protected:
    void executeTokenizer();

private:
    // [[nodiscard]] virtual auto getTextVector() const -> std::vector<icu::UnicodeString> = 0;
    [[nodiscard]] virtual auto getText() const -> utl::StringU8 = 0;
    std::string packName;
    PackId packId;
    std::size_t indexInPack;
    std::shared_ptr<WordDB> wordDB;
    std::shared_ptr<Tokenizer> tokenizer;

    CardId cardId{};
    std::vector<Token> tokens;
};

class DialogueCard : public Card
{
public:
    struct DialogueItem
    {
        utl::StringU8 speaker;
        utl::StringU8 text;
    };

    DialogueCard(std::string_view content,
                 const CardInit& cardInit);
    DialogueCard(const DialogueCard&) = default;
    DialogueCard(DialogueCard&&) = default;
    ~DialogueCard() override = default;
    auto operator=(const DialogueCard&) = delete;
    auto operator=(DialogueCard&&) = delete;

    [[nodiscard]] auto getDialogue() const -> const std::vector<DialogueItem>&;

    static auto deserialize(std::string_view content) -> std::vector<DialogueItem>;
    [[nodiscard]] auto serialize() const -> std::string override;

private:
    constexpr static auto s_prefix = "dlg";
    [[nodiscard]] auto getText() const -> utl::StringU8 override;
    std::vector<DialogueItem> dialogue;
};

class TextCard : public Card
{
public:
    TextCard(std::string_view content,
             const CardInit& cardInit);
    TextCard(const TextCard&) = default;
    TextCard(TextCard&&) = default;
    ~TextCard() override = default;
    auto operator=(const TextCard&) = delete;
    auto operator=(TextCard&&) = delete;

    static auto deserialize(std::string_view content) -> utl::StringU8;
    [[nodiscard]] auto serialize() const -> std::string override;

private:
    constexpr static auto s_prefix = "txt";
    [[nodiscard]] auto getText() const -> utl::StringU8 override;
    utl::StringU8 text;
};

} // namespace annotation
