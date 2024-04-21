#pragma once
#include "Token.h"
#include "WordDB.h"

#include <annotation/JieBa.h>
#include <annotation/ZH_Tokenizer.h>
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
         std::shared_ptr<annotation::JieBa> jieba);
    Card(const Card&) = default;
    Card(Card&&) = default;
    virtual ~Card() = default;
    auto operator=(const Card&) = delete;
    auto operator=(Card&&) = delete;

    [[nodiscard]] auto Id() const -> CardId;
    [[nodiscard]] auto getTokens() const -> const std::vector<Token>&;
    [[nodiscard]] virtual auto getTextVector() const -> std::vector<icu::UnicodeString> = 0;
    [[nodiscard]] virtual auto getText() const -> utl::StringU8 = 0;

private:
    std::string filename;
    CardId id;
    std::shared_ptr<annotation::WordDB> wordDB;
    std::shared_ptr<annotation::JieBa> jieba;

    std::vector<Token> tokens;
};

class DialogueCard : public Card
{
public:
    DialogueCard(std::string filename,
                 CardId id,
                 std::shared_ptr<WordDB> wordDB,
                 std::shared_ptr<annotation::JieBa> jieba);
    DialogueCard(const DialogueCard&) = default;
    DialogueCard(DialogueCard&&) = default;
    ~DialogueCard() override = default;
    auto operator=(const DialogueCard&) = delete;
    auto operator=(DialogueCard&&) = delete;

    struct DialogueItem
    {
        icu::UnicodeString speaker;
        icu::UnicodeString text;
    };

    std::vector<DialogueItem> dialogue;

    [[nodiscard]] auto getTextVector() const -> std::vector<icu::UnicodeString> override;
    [[nodiscard]] auto getText() const -> utl::StringU8 override;
};

class TextCard : public Card
{
public:
    TextCard(std::string filename,
             CardId id,
             std::shared_ptr<WordDB> wordDB,
             std::shared_ptr<annotation::JieBa> jieba);
    TextCard(const TextCard&) = default;
    TextCard(TextCard&&) = default;
    ~TextCard() override = default;
    auto operator=(const TextCard&) = delete;
    auto operator=(TextCard&&) = delete;

    icu::UnicodeString text;
    [[nodiscard]] auto getTextVector() const -> std::vector<icu::UnicodeString> override;
    [[nodiscard]] auto getText() const -> utl::StringU8 override;
};

} // namespace annotation
