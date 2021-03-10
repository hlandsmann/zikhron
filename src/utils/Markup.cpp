#include "Markup.h"
#include <fmt/format.h>
#include <iostream>
#include <limits>
#include <numeric>
#include <algorithm>
namespace ranges = std::ranges;

namespace markup {

auto Word::joinCharactersNonBreakable(const utl::StringU8& word) const -> std::string {
    std::string result = std::accumulate(
        word.cbegin(), std::prev(word.cend()), std::string{}, [](std::string&& a, const std::string& b) {
            return a += (b + "&#8288;");
        });
    result += word.back();
    return result;
}

auto Word::lengthOfWord(const utl::StringU8& word) const -> int { return word.length() * 2 - 1; }

Word::Word(const utl::StringU8& word, uint32_t _color, uint32_t _backGroundColor) {
    if (1 == 1 && word.front().isMarkup()) {
        markup = true;
        rawWord = word;
        styledWord = word;
        virtualLength = word.length();
    } else {
        virtualLength = lengthOfWord(word);
        color = _color;
        backGroundColor = _backGroundColor;
        rawWord = joinCharactersNonBreakable(word);
        styledWord = applyStyle(rawWord);
    }
}

Word::Word(const utl::StringU8& word) : Word{.word = word, ._color = 0, ._backGroundColor = 0} {}

Word& Word::operator=(const Word&& word) {
    this->~Word();
    new (this) Word(std::move(word));
    return *this;
}

Word::operator std::string() const { return styledWord; }

void Word::setColor(uint32_t _color) {
    if (markup)
        return;
    if (color == _color)
        return;
    color = _color;
    styledWord = applyStyle(rawWord);
}

void Word::setBackgroundColor(uint32_t _backgroundColor) {
    if (markup)
        return;
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

Paragraph::Paragraph(const Card& _card, const std::shared_ptr<ZH_Dictionary>& _zh_dictionary)
    : zh_dictionary(_zh_dictionary) {
    card = std::unique_ptr<Card>(_card.clone());
    utl::StringU8 text;
    if (DialogueCard* dlgCard = dynamic_cast<DialogueCard*>(card.get())) {
        const std::string tbOpen = "<tr>";
        const std::string tbClose = "</tr>";
        const std::string open = "<td style=\"padding:10px 15px 10px 15px;\">";
        const std::string close = "</td>";
        for (const auto& dialogue : dlgCard->dialogue) {
            text.push_back({tbOpen, true, 0});
            text.push_back({open, true, 1});
            // text.push_back({open,0});
            text.append(dialogue.speaker);
            text.push_back({close, true, 0});
            text.push_back({open, true, 1});
            text.append(dialogue.text);
            text.push_back({close, true, 0});
            text.push_back({tbClose, true, 0});
        }
    }
    if (TextCard* textCard = dynamic_cast<TextCard*>(card.get())) {
        text = textCard->text;
    }
    zh_annotator = std::make_unique<ZH_Annotator>(text, zh_dictionary);
    ranges::transform(zh_annotator->Items(),
                      std::back_inserter(*this),
                      [](const ZH_Annotator::Item &item) -> markup::Word {
                          std::cout << item.text << " : " << item.text.length() << "\n";
                          if (not item.dicItemVec.empty())
                              return {.word = item.text, .color = 0, .backGroundColor = 0x010101};
                          return item.text;
                      });
}

auto Paragraph::get() const -> std::string {
    return std::accumulate(words.begin(), words.end(), std::string{}, utl::stringPlusT<Word>);
}

void Paragraph::push_back(const Word& word) {
    if (positions.empty())
        positions.push_back(0);
    else
        positions.push_back(positions.back() + words.back().vLength());
    words.push_back(word);
}

void Paragraph::resetPosition() {
    std::transform(words.begin(),
                   words.end(),
                   std::back_inserter(positions),
                   [absolutePosition = 0](const Word& word) mutable {
                       absolutePosition += word.vLength();
                       return absolutePosition - word.vLength();
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
    if (words[index].isMarkup())
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
