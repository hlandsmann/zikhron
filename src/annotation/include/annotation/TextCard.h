#pragma once
#include <annotation/ZH_Annotator.h>
#include <sys/types.h>
#include <unicode/unistr.h>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct Card {
    Card(std::string _filename, uint _id) : filename(std::move(_filename)), id(_id){};
    [[nodiscard]] virtual auto clone() const -> std::unique_ptr<Card> = 0;
    virtual ~Card() = default;

    [[nodiscard]] virtual auto getTextVector() const -> std::vector<icu::UnicodeString> = 0;
    [[nodiscard]] uint getId() const { return id; }

    std::optional<ZH_Annotator> zh_annotator;
    auto getAnnotator() -> ZH_Annotator& {
        if (not zh_annotator.has_value()) {
            zh_annotator.emplace();
        }
        return zh_annotator.value();
    }

private:
    std::string filename;
    uint id;
};

template <typename Card_Type> struct Card_clone : public Card {
public:
    Card_clone(std::string _filename, uint _id) : Card(std::move(_filename), _id){};
    [[nodiscard]] std::unique_ptr<Card> clone() const override {
        return std::make_unique<Card_Type>(static_cast<Card_Type const&>(*this));
    }
};

struct DialogueCard : public Card_clone<DialogueCard> {
    DialogueCard(const std::string& _filename, uint _id) : Card_clone<DialogueCard>(_filename, _id){};
    DialogueCard(const DialogueCard&) = default;
    struct DialogueItem {
        icu::UnicodeString speaker;
        icu::UnicodeString text;
    };
    std::vector<DialogueItem> dialogue;

    [[nodiscard]] auto getTextVector() const -> std::vector<icu::UnicodeString> override;
};

struct TextCard : public Card_clone<TextCard> {
    TextCard(std::string _filename, uint _id) : Card_clone<TextCard>(std::move(_filename), _id){};
    TextCard(const TextCard&) = default;
    icu::UnicodeString text;
    [[nodiscard]] auto getTextVector() const -> std::vector<icu::UnicodeString> override;
};

class CardDB {
public:
    using CardPtr = std::unique_ptr<Card>;

    void loadFromSingleJson(std::string jsonFileName);
    void loadFromDirectory(std::string directoryPath);

    [[nodiscard]] auto get() const -> const std::map<uint, CardPtr>&;

private:
    std::map<uint, CardPtr> cards;
};
