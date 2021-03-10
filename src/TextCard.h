#pragma once
#include <unicode/unistr.h>
#include <memory>
#include <string>
#include <vector>

struct Card {
    Card(std::string _filename = "") : filename(_filename){};
    virtual auto clone() const -> Card* = 0;
    virtual ~Card() = default;

    virtual auto getTextVector() const -> std::vector<icu::UnicodeString> = 0;
    const std::string filename;
};

struct DialogueCard : public Card {
    DialogueCard(std::string _filename = "") : Card(_filename){};
    DialogueCard(const DialogueCard&) = default;
    auto clone() const -> DialogueCard* override { return new DialogueCard(*this); }
    struct DialogueItem {
        icu::UnicodeString speaker;
        icu::UnicodeString text;
    };
    std::vector<DialogueItem> dialogue;

    auto getTextVector() const -> std::vector<icu::UnicodeString> override;
};

struct TextCard : public Card {
    TextCard(std::string _filename = "") : Card(_filename){};
    auto clone() const -> TextCard* override { return new TextCard(*this); }
    TextCard(const TextCard&) = default;
    icu::UnicodeString text;
    auto getTextVector() const -> std::vector<icu::UnicodeString> override;
};

class CardDB {
public:
    using CardPtr = std::unique_ptr<Card>;

    void loadFromSingleJson(std::string jsonFileName);
    void loadFromDirectory(std::string directoryPath);

    auto get() const -> const std::vector<CardPtr>&;
    auto moveOut(int index) -> CardPtr;

private:
    std::vector<CardPtr> cards;
};
