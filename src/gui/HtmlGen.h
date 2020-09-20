#pragma once
#include <utils/StringU8.h>
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
    auto Get() const -> std::string;
    void push_back(const Word& word);

private:
    std::vector<Word> words;
};
}  // namespace markup

inline std::ostream& operator<<(std::ostream& os, const markup::Word& word) {
    os << std::string(word);
    return os;
}

inline std::string operator+(std::string&& str, const markup::Word &word) {
    return str + std::string(word);
}
// inline std::string& operator+=(std::string& str, const markup::Word& word) {
//     str += std::string(word);
//     return str;
// }
