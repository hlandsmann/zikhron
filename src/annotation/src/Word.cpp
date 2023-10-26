#include <CardDB.h>
#include <Ease.h>
#include <Word.h>
#include <ZH_Annotator.h>
#include <fmt/format.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <utils/Memoizer.h>
#include <utils/StringU8.h>

#include <boost/range/combine.hpp>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iterator>
#include <numeric>
#include <string>

#include <sys/types.h>

namespace ranges = std::ranges;

namespace markup {

auto Word::joinCharactersNonBreakable(const utl::StringU8& word) -> std::string
{
    std::string result = std::accumulate(
            word.cbegin(), std::prev(word.cend()), std::string{}, [](std::string&& a, const std::string& b) {
                return a += (b + "&#8288;");
            });
    result += word.back();
    return result;
}

auto Word::utf8ByteLengthOfWord(const utl::StringU8& word) -> size_t
{
    size_t result = std::accumulate(
            word.cbegin(), std::prev(word.cend()), size_t{}, [](size_t acc, const std::string& str) {
                return acc + str.length() + 3; // 3 is the utf8 length of the non breakable character &#8288
            });
    result += std::string(word.back()).length();
    return result;
}

auto Word::lengthOfWord(const utl::StringU8& word) -> size_t
{
    return word.vlength() * 2 - 1;
}

Word::Word(const utl::StringU8& word, uint32_t _color, uint32_t _backGroundColor)
{
    if (word.front().isMarkup()) {
        markup = true;
        rawWord = word;
        styledWord = word;
        virtualLength = word.vlength();
        utf8ByteLength = std::string(word).length();
    } else {
        virtualLength = lengthOfWord(word);
        utf8ByteLength = utf8ByteLengthOfWord(word);
        color = _color;
        backGroundColor = _backGroundColor;
        rawWord = joinCharactersNonBreakable(word);
        styledWord = applyStyle(rawWord);
    }
}

Word::Word(const utl::StringU8& word)
    : Word(word, /*color*/ 0, /* background_color*/ 0) {}

Word::operator std::string() const
{
    return styledWord;
}

void Word::setColor(uint32_t _color)
{
    if (markup) {
        return;
    }
    if (color == _color) {
        return;
    }
    color = _color;
    styledWord = applyStyle(rawWord);
}

void Word::setBackgroundColor(uint32_t _backgroundColor)
{
    if (markup) {
        return;
    }
    if (backGroundColor == _backgroundColor) {
        return;
    }
    backGroundColor = _backgroundColor;
    styledWord = applyStyle(rawWord);
}

auto Word::applyStyle(const std::string& str) const -> std::string
{
    constexpr uint32_t colorMask = 0x00FFFFFF;

    std::string style{};
    if ((colorMask & color) != 0) {
        style = std::format("{} color=\"#{:06x}\"", style, (colorMask & color));
    }
    if ((colorMask & backGroundColor) != 0) {
        style = std::format("{} background=\"#{:06x}\"", style, (colorMask & backGroundColor));
    }
    if (not style.empty()) {
        return std::format("<span{}>{}</span>", style, str);
    }
    return str;
}

} // namespace markup
