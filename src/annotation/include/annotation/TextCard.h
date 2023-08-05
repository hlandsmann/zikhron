#pragma once
#include <annotation/ZH_Annotator.h>
#include <unicode/unistr.h>
#include <utils/StringU8.h>

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <sys/types.h>

struct BaseCard
{
    BaseCard(std::string _filename, uint _id)
        : filename(std::move(_filename)), id(_id){};
    BaseCard(const BaseCard&) = default;
    BaseCard(BaseCard&&) = default;
    virtual ~BaseCard() = default;
    auto operator=(const BaseCard&) = delete;
    auto operator=(BaseCard&&) = delete;

    [[nodiscard]] virtual auto clone() const -> std::unique_ptr<BaseCard> = 0;

    [[nodiscard]] virtual auto getTextVector() const -> std::vector<icu::UnicodeString> = 0;
    [[nodiscard]] auto getId() const -> unsigned { return id; }
    [[nodiscard]] virtual auto getText() const -> utl::StringU8 = 0;

    using CharacterSequence = std::vector<utl::CharU8>;
    using Combination = std::vector<int>;
    auto createAnnotator(const std::shared_ptr<const ZH_Dictionary>& _dictionary,
                         const std::map<CharacterSequence, Combination>& _choices = {})
            -> ZH_Annotator&
    {
        zh_annotator.emplace(getText(), _dictionary, _choices);
        return zh_annotator.value();
    }
    auto getAnnotator() -> ZH_Annotator&
    {
        if (not zh_annotator.has_value()) {
            zh_annotator.emplace();
        }
        return zh_annotator.value();
    }

private:
    std::optional<ZH_Annotator> zh_annotator;
    std::string filename;
    uint id;
};

template<typename Card_Type>
struct Card_clone : public BaseCard
{
public:
    Card_clone(std::string _filename, uint _id)
        : BaseCard(std::move(_filename), _id){};
    Card_clone(const Card_clone&) = default;
    Card_clone(Card_clone&&) = default;
    ~Card_clone() override = default;
    auto operator=(const Card_clone&) = delete;
    auto operator=(Card_clone&&) = delete;

    [[nodiscard]] auto clone() const -> std::unique_ptr<BaseCard> override
    {
        return std::make_unique<Card_Type>(static_cast<Card_Type const&>(*this));
    }
};

struct DialogueCard : public Card_clone<DialogueCard>
{
    DialogueCard(const std::string& _filename, uint _id)
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
    TextCard(std::string _filename, uint _id)
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

    using CardPtr = std::unique_ptr<BaseCard>;
    using CharacterSequence = std::vector<utl::CharU8>;
    using Combination = std::vector<int>;

    CardDB() = default;
    CardDB(const std::filesystem::path& directoryPath,
           const std::shared_ptr<const ZH_Dictionary>& dictionary,
           const std::map<CharacterSequence, Combination>& choices);
    void loadFromDirectory(const std::filesystem::path& directoryPath);

    [[nodiscard]] auto get() const -> const std::map<uint, CardPtr>&;

private:
    std::map<uint, CardPtr> cards;
};
