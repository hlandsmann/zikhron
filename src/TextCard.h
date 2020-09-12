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
    struct DialogueItem {
        icu::UnicodeString speaker;
        icu::UnicodeString text;
    };
    std::vector<DialogueItem> dialogue;

    auto getTextVector() const -> std::vector<icu::UnicodeString> override;
};

struct TextCard : public Card {
    icu::UnicodeString text;
    auto getTextVector() const -> std::vector<icu::UnicodeString> override;
};

struct CardDB {
    void loadFromSingleJson(std::string jsonFileName);
    using CardPtr = std::unique_ptr<Card>;
    std::vector<CardPtr> cards;
};
