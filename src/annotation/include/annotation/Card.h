#pragma once
#include "Token.h"
#include "Tokenizer.h"
#include "WordDB.h"

#include <annotation/JieBa.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Identifier.h>
#include <unicode/unistr.h>
#include <utils/StringU8.h>

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <sys/types.h>

namespace annotation {

class Card
{
public:
    using CharacterSequence = std::vector<utl::CharU8>;

    Card(std::string filename,
         CardId id,
         std::shared_ptr<WordDB> wordDB,
         std::shared_ptr<annotation::Tokenizer> tokenizer);
    Card(const Card&) = default;
    Card(Card&&) = default;
    virtual ~Card() = default;
    auto operator=(const Card&) = delete;
    auto operator=(Card&&) = delete;

    [[nodiscard]] auto Id() const -> CardId;
    [[nodiscard]] auto getTokens() const -> const std::vector<Token>&;
    [[nodiscard]] auto getWordDB() const -> std::shared_ptr<WordDB>;
    void getAlternatives();

protected:
    void executeJieba();

private:
    // [[nodiscard]] virtual auto getTextVector() const -> std::vector<icu::UnicodeString> = 0;
    [[nodiscard]] virtual auto getText() const -> utl::StringU8 = 0;
    std::string filename;
    CardId id;
    std::shared_ptr<WordDB> wordDB;
    std::shared_ptr<Tokenizer> tokenizer;

    std::vector<Token> tokens;
};

class DialogueCard : public Card
{
public:
    struct DialogueItem
    {
        icu::UnicodeString speaker;
        icu::UnicodeString text;
    };

    DialogueCard(std::string filename,
                 CardId id,
                 std::shared_ptr<WordDB> wordDB,
                 std::shared_ptr<Tokenizer> tokenizer,
                 std::vector<DialogueItem>&& dialogue);
    DialogueCard(const DialogueCard&) = default;
    DialogueCard(DialogueCard&&) = default;
    ~DialogueCard() override = default;
    auto operator=(const DialogueCard&) = delete;
    auto operator=(DialogueCard&&) = delete;

    [[nodiscard]] auto getDialogue() const -> const std::vector<DialogueItem>&;

private:
    // [[nodiscard]] auto getTextVector() const -> std::vector<icu::UnicodeString> override;
    [[nodiscard]] auto getText() const -> utl::StringU8 override;
    std::vector<DialogueItem> dialogue;
};

class TextCard : public Card
{
public:
    TextCard(std::string filename,
             CardId id,
             std::shared_ptr<WordDB> wordDB,
             std::shared_ptr<Tokenizer> tokenizer,
             icu::UnicodeString text);
    TextCard(const TextCard&) = default;
    TextCard(TextCard&&) = default;
    ~TextCard() override = default;
    auto operator=(const TextCard&) = delete;
    auto operator=(TextCard&&) = delete;

private:
    // [[nodiscard]] auto getTextVector() const -> std::vector<icu::UnicodeString> override;
    [[nodiscard]] auto getText() const -> utl::StringU8 override;
    icu::UnicodeString text;
};

} // namespace annotation
