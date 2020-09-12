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
    Word(const utl::StringU8& word);
    operator std::string() const;

    void setColor(uint32_t color);
    void setBackgroundColor(uint32_t backgroundColor);

    const int length;

private:
    auto applyStyle(const std::string& str) const -> std::string;
    uint32_t color = 0;
    uint32_t backGroundColor = 0;
    std::string styledWord;
};

class Paragraph {
public:
    Paragraph() = default;

private:
    std::vector<utl::StringU8> words;
};
}  // namespace markup

inline std::ostream& operator<<(std::ostream& os, const markup::Word& word) {
    os << std::string(word);
    return os;
}
