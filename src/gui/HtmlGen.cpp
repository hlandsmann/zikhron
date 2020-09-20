#include "HtmlGen.h"
#include <fmt/format.h>
#include <numeric>

namespace markup {

auto Word::joinCharactersNonBreakable(const utl::StringU8& word) const -> std::string {
    std::string result;
    word.iterate_characters(
        [&result](const std::string_view& sv) { result += std::string(sv) + "&#8288;"; }, 0, -1);
    result += word.back();

    return result;
}

auto Word::lengthOfWord(const utl::StringU8& word) const -> int { return word.length() * 2 - 1; }
Word::Word(const utl::StringU8& word, uint32_t _color, uint32_t _backGroundColor)
    : rawWord(joinCharactersNonBreakable(word))
    , virtualLength(lengthOfWord(word))
    , color(_color)
    , backGroundColor(_backGroundColor)
    , styledWord(applyStyle(rawWord)) {}

Word::Word(const utl::StringU8& word)
    : rawWord(joinCharactersNonBreakable(word))
    , virtualLength(lengthOfWord(word))
    , styledWord(applyStyle(rawWord)) {}

Word::operator std::string() const { return styledWord; }

void Word::setColor(uint32_t _color) {
    if (color == _color)
        return;
    color = _color;
    styledWord = applyStyle(rawWord);
}

void Word::setBackgroundColor(uint32_t _backgroundColor) {
    if (backGroundColor == _backgroundColor)
        return;
    backGroundColor = _backgroundColor;
    styledWord = applyStyle(rawWord);
}

auto Word::applyStyle(const std::string& str) const -> std::string {
    constexpr uint32_t colorMask = 0x00FFFFFF;

    std::string style = "";
    if ((colorMask & color) != 0)
        style = fmt::format("{}color:#{:06x};", style, (colorMask & color));
    if ((colorMask & backGroundColor) != 0)
        style = fmt::format("{}background-color:#{:06x};", style, (colorMask & backGroundColor));
    if (not style.empty())
        return fmt::format("<span style=\"{}\">{}</span>", style, str);
    return str;
}

auto Paragraph::Get() const -> std::string {
    return std::accumulate(
        words.begin(), words.end(), std::string{}, [](std::string&& a, const Word& b) {
            return a += b;
        });
}

void Paragraph::push_back(const Word& word) { words.push_back(word); }

}  // namespace markup
