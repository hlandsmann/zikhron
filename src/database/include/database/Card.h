#pragma once
#include "CbdFwd.h"
#include "WordDB.h"

#include <annotation/JieBa.h>
#include <annotation/Token.h>
#include <annotation/Tokenizer.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Identifier.h>
#include <misc/TokenizationChoice.h>
#include <unicode/unistr.h>
#include <utils/StringU8.h>

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <sys/types.h>

namespace database {

struct CardInit
{
    CardId cardId;
    std::string packName;
    PackId packId;
    std::size_t indexInPack;
    std::shared_ptr<database::WordDB> wordDB;
    std::shared_ptr<annotation::Tokenizer> tokenizer;
};

class Card
{
public:
    using CharacterSequence = std::vector<utl::CharU8>;
    static auto deserializeCard(std::string_view content, const CardInit& cardInit) -> CardPtr;

    Card(const CardInit& cardInit);
    virtual ~Card() = default;

    Card(const Card&) = delete;
    Card(Card&&) = delete;
    auto operator=(const Card&) = delete;
    auto operator=(Card&&) = delete;

    [[nodiscard]] auto getCardId() const -> CardId;
    [[nodiscard]] auto getTokens() const -> const std::vector<annotation::Token>&;
    [[nodiscard]] auto getWordDB() const -> std::shared_ptr<database::WordDB>;
    [[nodiscard]] auto getTokenizer() const -> std::shared_ptr<annotation::Tokenizer>;
    [[nodiscard]] auto getAlternatives() const -> std::vector<annotation::Alternative>;
    [[nodiscard]] virtual auto serialize() const -> std::string = 0;
    [[nodiscard]] auto getPackId() const -> PackId;
    [[nodiscard]] auto getPackName() const -> std::string;
    [[nodiscard]] auto getIndexInPack() const -> std::size_t;
    void setTokenizationChoices(const TokenizationChoiceVec& tokenizationChoices);

protected:
    void executeTokenizer();

private:
    [[nodiscard]] virtual auto getText() const -> utl::StringU8 = 0;

    CardId cardId;

    std::string packName;
    PackId packId;
    std::size_t indexInPack;

    std::shared_ptr<database::WordDB> wordDB;
    std::shared_ptr<annotation::Tokenizer> tokenizer;

    std::vector<annotation::Token> tokens;
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
    DialogueCard(const DialogueCard&) = delete;
    DialogueCard(DialogueCard&&) = delete;
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
    TextCard(const TextCard&) = delete;
    TextCard(TextCard&&) = delete;
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

class SubtitleCard : public Card
{
public:
    SubtitleCard(std::vector<std::string> content,
                 const CardInit& cardInit);
    SubtitleCard(const SubtitleCard&) = delete;
    SubtitleCard(SubtitleCard&&) = delete;
    ~SubtitleCard() override = default;
    auto operator=(const SubtitleCard&) = delete;
    auto operator=(SubtitleCard&&) = delete;

    [[nodiscard]] auto serialize() const -> std::string override { return {}; };

private:
    [[nodiscard]]  auto getText() const -> utl::StringU8 override;
    std::vector<utl::StringU8> joinedSubs;
};

} // namespace database
