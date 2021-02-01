#include "Markup.h"
#include <fmt/format.h>
#include <iostream>
#include <limits>
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

Word::Word(const utl::StringU8& word) : Word{.word = word, ._color = 0, ._backGroundColor = 0} {}

Word& Word::operator=(const Word&& word) {
    this->~Word();
    new (this) Word(std::move(word));
    return *this;
}

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

auto Paragraph::get() const -> std::string {
    return std::accumulate(
        words.begin(), words.end(), std::string{}, [](std::string&& a, const Word& b) {
            return a += b;
        });
}

void Paragraph::push_back(const Word& word) {
    if (positions.empty())
        positions.push_back(0);
    else
        positions.push_back(positions.back() + words.back().virtualLength);
    words.push_back(word);
}

void Paragraph::resetPosition() {
    int absolutePosition = 0;
    std::transform(words.begin(),
                   words.end(),
                   std::back_inserter(positions),
                   [&absolutePosition](const Word& word) {
                       absolutePosition += word.virtualLength;
                       return absolutePosition - word.virtualLength;
                   });
}

auto Paragraph::getWordStartPosition(int pos) const -> int {
    std::size_t index = getWordIndex(pos);
    if (index >= positions.size())
        return -1;
    return positions[index];
}

auto Paragraph::getWordIndex(int pos) const -> std::size_t {
    auto posIt = std::adjacent_find(positions.begin(), positions.end(), [pos](int posA, int posB) {
        return posA <= pos && posB > pos;
    });
    if (posIt == positions.end()) {
        if (pos > positions.back() || pos < 0)
            return std::numeric_limits<std::size_t>::max();
        posIt = std::prev(positions.end());
    }
    return std::max<int>(0, std::distance(positions.begin(), posIt));
}

void Paragraph::changeWordAtPosition(int pos, const std::function<void(Word&)>& op) {
    if (positions.empty())
        return;
    int index = getWordIndex(pos);

    changeWordAtIndex(index, op);
}

void Paragraph::changeWordAtIndex(std::size_t index, const std::function<void(Word&)>& op) {
    if (index >= words.size())
        return;
    preChanges.push({.index = index, .word{words[index]}});
    op(words[index]);
}

void Paragraph::undoChange() {
    if (preChanges.empty())
        return;
    WordState preChange = preChanges.top();
    preChanges.pop();
    words[preChange.index] = std::move(preChange.word);
}

}  // namespace markup
