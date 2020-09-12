#include "HtmlGen.h"
#include <fmt/format.h>

namespace markup {

auto Word::joinCharactersNonBreakable(const utl::StringU8& word) const -> std::string {
    std::string result;
    word.iterate_characters(
        [&result](const std::string_view& sv) { result += std::string(sv) + "&#8288;"; }, 0, -1);
    result += word.back();

    return result;
}

auto Word::lengthOfWord(const utl::StringU8& word) const -> int { return word.length() * 2 - 1; }

Word::Word(const utl::StringU8& word)
    : rawWord(joinCharactersNonBreakable(word))
    , length(lengthOfWord(word))
    , styledWord(applyStyle(rawWord)) {}

Word::operator std::string() const { return styledWord; }

void Word::setColor(uint32_t _color) {
    color = _color;
    styledWord = applyStyle(rawWord);
}

void Word::setBackgroundColor(uint32_t _backgroundColor) {
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

}  // namespace markup
