#pragma once
#include <utils/StringU8.h>
#include <stack>
#include <string>
#include <vector>

namespace markup {

class Word {
    const std::string rawWord;
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

    const int virtualLength;

private:
    auto applyStyle(const std::string& str) const -> std::string;
    uint32_t color = 0;
    uint32_t backGroundColor = 0;
    std::string styledWord;
};

class Paragraph {
public:
    using value_type = Word;

    Paragraph() = default;
    auto get() const -> std::string;
    void push_back(const Word& word);
    void changeWordAtPosition(int pos, const std::function<void(Word&)>& op);
    void changeWordAtIndex(int index, const std::function<void(Word&)>& op);
    void undoChange();

private:
    void resetPosition();
    struct WordState {
        int index;
        Word word;
    };

    std::stack<WordState> preChanges;
    std::vector<Word> words;
    std::vector<int> positions;
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
