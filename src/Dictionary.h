#pragma once

#include <unicode/unistr.h>
#include <memory>
#include <string>
#include <vector>

class Dictionary {
public:
    Dictionary() = default;
    void loadFromJson(std::string jsonFileName);

    struct WordValue {
        std::vector<icu::UnicodeString> meanings;
        std::vector<icu::UnicodeString> pronounciations;
        std::vector<icu::UnicodeString> alternatives;
    };
    struct Word {
        Word(const Word &word);
        Word(icu::UnicodeString key, std::unique_ptr<WordValue> value);
        icu::UnicodeString         key;
        std::unique_ptr<WordValue> value;
    };

    const std::vector<Word>& GetWords() const { return words; }

// private:
    std::vector<Word> words;
};
