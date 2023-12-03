#pragma once

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

class Card
{
public:
    using CharacterSequence = std::vector<utl::CharU8>;
    using Combination = std::vector<int>;
    using AnnotationChoiceMap = std::map<CharacterSequence, Combination>;

    Card(std::string filename,
         CardId id,
         std::shared_ptr<const ZH_Dictionary> dictionary,
         std::shared_ptr<const AnnotationChoiceMap> annotationChoices);
    Card(const Card&) = default;
    Card(Card&&) = default;
    virtual ~Card() = default;
    auto operator=(const Card&) = delete;
    auto operator=(Card&&) = delete;

    [[nodiscard]] auto Id() const -> CardId;
    [[nodiscard]] virtual auto getTextVector() const -> std::vector<icu::UnicodeString> = 0;
    [[nodiscard]] virtual auto getText() const -> utl::StringU8 = 0;

    [[nodiscard]] auto getTokenizer() const -> ZH_Tokenizer&;
    void resetTokenizer();

private:
    std::string filename;
    CardId id;
    mutable std::optional<ZH_Tokenizer> tokenizer;
    std::shared_ptr<const ZH_Dictionary> dictionary;
    std::shared_ptr<const AnnotationChoiceMap> annotationChoices;
};

class DialogueCard : public Card
{
public:
    DialogueCard(const std::string& filename,
                 CardId id,
                 std::shared_ptr<const ZH_Dictionary> dictionary,
                 std::shared_ptr<const AnnotationChoiceMap> annotationChoices);
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
    TextCard(const std::string& filename,
             CardId id,
             std::shared_ptr<const ZH_Dictionary> dictionary,
             std::shared_ptr<const AnnotationChoiceMap> annotationChoices);
    TextCard(const TextCard&) = default;
    TextCard(TextCard&&) = default;
    ~TextCard() override = default;
    auto operator=(const TextCard&) = delete;
    auto operator=(TextCard&&) = delete;

    icu::UnicodeString text;
    [[nodiscard]] auto getTextVector() const -> std::vector<icu::UnicodeString> override;
    [[nodiscard]] auto getText() const -> utl::StringU8 override;
};
