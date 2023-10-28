#pragma once
#include <annotation/ZH_Annotator.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Identifier.h>
#include <unicode/unistr.h>
#include <utils/StringU8.h>

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
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

    auto getAnnotator() -> ZH_Annotator&;
    void resetAnnotator();

private:
    std::string filename;
    CardId id;
    std::optional<ZH_Annotator> annotator;
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

class CardDB
{
public:
    static constexpr std::string_view s_cardSubdirectory = "cards";

    using CardPtr = std::shared_ptr<Card>;
    using CardPtrConst = std::shared_ptr<const Card>;
    using CharacterSequence = Card::CharacterSequence;
    using Combination = Card::Combination;
    using AnnotationChoiceMap = Card::AnnotationChoiceMap;

    CardDB() = default;
    CardDB(const std::filesystem::path& directoryPath,
           std::shared_ptr<const ZH_Dictionary> dictionary,
           std::shared_ptr<const AnnotationChoiceMap> annotationChoices);
    static auto loadFromDirectory(const std::filesystem::path& directoryPath,
                                  const std::shared_ptr<const ZH_Dictionary>& dictionary,
                                  const std::shared_ptr<const AnnotationChoiceMap>& annotationChoices)
            -> std::map<CardId, CardPtr>;

    [[nodiscard]] auto get() const -> const std::map<CardId, CardPtr>&;
    [[nodiscard]] auto atId(CardId) -> CardPtr&;
    [[nodiscard]] auto atId(CardId) const -> CardPtrConst;

private:
    std::shared_ptr<const ZH_Dictionary> dictionary;
    std::shared_ptr<const AnnotationChoiceMap> annotationChoices;
    std::map<CardId, CardPtr> cards;
};
