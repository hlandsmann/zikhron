#pragma once
#include <unicode/unistr.h>
#include <memory>
#include <string>
#include <vector>

struct Card {
    virtual ~Card() = default;

    virtual auto getTextVector() const -> std::vector<icu::UnicodeString> = 0;
};
struct DialogueCard : public Card {
    DialogueCard() = default;
    DialogueCard(const DialogueCard&) = default;
    struct DialogueItem {
        icu::UnicodeString speaker;
        icu::UnicodeString text;
    };
    std::vector<DialogueItem> dialogue;

    auto getTextVector() const -> std::vector<icu::UnicodeString> override;
};

struct TextCard : public Card {
    TextCard() = default;
    TextCard(const TextCard&) = default;
    icu::UnicodeString text;
    auto getTextVector() const -> std::vector<icu::UnicodeString> override;
};

class CardDB {
public:
    using CardPtr = std::unique_ptr<Card>;

    void loadFromSingleJson(std::string jsonFileName);

    auto get() const -> const std::vector<CardPtr>&;
    auto moveOut(int index) -> CardPtr;
private:
    std::vector<CardPtr> cards;
};
