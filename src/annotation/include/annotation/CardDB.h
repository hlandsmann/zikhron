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
#include <utility>
#include <vector>

#include <sys/types.h>

struct Card
{
    Card(std::string _filename, CardId _id)
        : filename(std::move(_filename)), id(_id){};
    Card(const Card&) = default;
    Card(Card&&) = default;
    virtual ~Card() = default;
    auto operator=(const Card&) = delete;
    auto operator=(Card&&) = delete;

    [[nodiscard]] virtual auto clone() const -> std::unique_ptr<Card> = 0;

    [[nodiscard]] auto Id() const -> CardId;
    [[nodiscard]] virtual auto getTextVector() const -> std::vector<icu::UnicodeString> = 0;
    [[nodiscard]] virtual auto getText() const -> utl::StringU8 = 0;

    using CharacterSequence = std::vector<utl::CharU8>;
    using Combination = std::vector<int>;
    auto createAnnotator(const std::shared_ptr<const ZH_Dictionary>& _dictionary,
                         const std::map<CharacterSequence, Combination>& _choices = {}) -> ZH_Annotator&;
    auto getAnnotator() -> ZH_Annotator&;

private:
    std::optional<ZH_Annotator> zh_annotator;
    std::string filename;
    CardId id;
};

template<typename Card_Type>
struct Card_clone : public Card
{
public:
    Card_clone(std::string _filename, CardId _id)
        : Card(std::move(_filename), _id){};
    Card_clone(const Card_clone&) = default;
    Card_clone(Card_clone&&) = default;
    ~Card_clone() override = default;
    auto operator=(const Card_clone&) = delete;
    auto operator=(Card_clone&&) = delete;

    [[nodiscard]] auto clone() const -> std::unique_ptr<Card> override
    {
        return std::make_unique<Card_Type>(static_cast<Card_Type const&>(*this));
    }
};

struct DialogueCard : public Card_clone<DialogueCard>
{
    DialogueCard(const std::string& _filename, CardId _id)
        : Card_clone<DialogueCard>(_filename, _id){};
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

struct TextCard : public Card_clone<TextCard>
{
    TextCard(std::string _filename, CardId _id)
        : Card_clone<TextCard>(std::move(_filename), _id){};
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
    using CharacterSequence = std::vector<utl::CharU8>;
    using Combination = std::vector<int>;

    CardDB() = default;
    CardDB(const std::filesystem::path& directoryPath,
           const std::shared_ptr<const ZH_Dictionary>& dictionary,
           const std::map<CharacterSequence, Combination>& choices);
    void loadFromDirectory(const std::filesystem::path& directoryPath);

    [[nodiscard]] auto get() const -> const std::map<CardId, CardPtr>&;

private:
    std::map<CardId, CardPtr> cards;
};
