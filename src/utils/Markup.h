#pragma once
#include <TextCard.h>
#include <utils/StringU8.h>
#include <stack>
#include <string>
#include <vector>

#include <ZH_Annotator.h>
#include <ZH_Dictionary.h>
#include <iostream>
namespace markup {

class Word {
    std::string rawWord;
    auto joinCharactersNonBreakable(const utl::StringU8& word) const -> std::string;
    auto lengthOfWord(const utl::StringU8& word) const -> int;

public:
    Word(const utl::StringU8& word, uint32_t color, uint32_t backGroundColor);
    Word(const utl::StringU8& word);
    Word(const Word&) = default;
    Word(Word&&) = default;

    Word& operator=(const Word&& word);
    operator std::string() const;

    void setColor(uint32_t color);
    void setBackgroundColor(uint32_t backgroundColor);
    auto vLength() const -> int { return virtualLength; }
    auto isMarkup() const -> bool { return markup; }

private:
    auto applyStyle(const std::string& str) const -> std::string;
    int virtualLength = 0;
    bool markup = false;
    uint32_t color = 0;
    uint32_t backGroundColor = 0;
    std::string styledWord;
};

class Paragraph {
public:
    using value_type = Word;
    static utl::StringU8 textFromCard(const Card&);

    Paragraph() = default;
    Paragraph(const Card&, const std::shared_ptr<ZH_Dictionary>&);

    auto get() const -> std::string;
    void push_back(const Word& word);
    auto getWordStartPosition(int pos) const -> int;
    auto getWordIndex(int pos) const -> std::size_t;

    void changeWordAtPosition(int pos, const std::function<void(Word&)>& op);
    void changeWordAtIndex(std::size_t index, const std::function<void(Word&)>& op);
    void undoChange();
    auto wordFromPosition(int pos) const -> const ZH_Annotator::ZH_dicItemVec;
    void setupVocables(std::vector<ZH_Dictionary::Item>&&);
    auto getVocableString() const -> std::string;

private:
    void resetPosition();
    struct WordState {
        std::size_t index;
        Word word;
    };
    std::unique_ptr<ZH_Annotator> zh_annotator;
    std::shared_ptr<ZH_Dictionary> zh_dictionary;
    // std::unique_ptr<Card> card;

    std::stack<WordState> preChanges;
    std::vector<Word> words;
    std::vector<int> positions;

    std::vector<ZH_Dictionary::Item> vocables;
    std::string vocableString;
};
}  // namespace markup

inline std::ostream& operator<<(std::ostream& os, const markup::Word& word) {
    os << std::string(word);
    return os;
}

inline std::string operator+(std::string&& str, const markup::Word& word) {
    return str + std::string(word);
}
// inline std::string& operator+=(std::string& str, const markup::Word& word) {
//     str += std::string(word);
//     return str;
// }
