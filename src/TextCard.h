#pragma once
#include <unicode/unistr.h>
#include <memory>
#include <string>
#include <vector>

struct Card {
    virtual ~Card() = default;

    virtual std::vector<icu::UnicodeString> getTextVector() const = 0;
};
struct DialogueCard : public Card {
    struct DialogueItem {
        icu::UnicodeString speaker;
        icu::UnicodeString text;
    };
    std::vector<DialogueItem> dialogue;

    std::vector<icu::UnicodeString> getTextVector() const override;
};

struct TextCard : public Card {
    icu::UnicodeString              text;
    std::vector<icu::UnicodeString> getTextVector() const override;
};

struct CardDB {
    void loadFromSingleJson(std::string jsonFileName);
    using CardPtr = std::unique_ptr<Card>;
    std::vector<CardPtr> cards;
};
