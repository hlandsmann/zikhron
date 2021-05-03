#include "Markup.h"
#include <fmt/format.h>
#include <algorithm>
#include <boost/range/combine.hpp>
#include <iostream>
#include <limits>
#include <numeric>
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

auto Word::lengthOfWord(const utl::StringU8& word) const -> int { return word.vlength() * 2 - 1; }

Word::Word(const utl::StringU8& word, uint32_t _color, uint32_t _backGroundColor) {
    if (1 == 1 && word.front().isMarkup()) {
        markup = true;
        rawWord = word;
        styledWord = word;
        virtualLength = word.vlength();
    } else {
        virtualLength = lengthOfWord(word);
        color = _color;
        backGroundColor = _backGroundColor;
        rawWord = joinCharactersNonBreakable(word);
        styledWord = applyStyle(rawWord);
    }
}

Word::Word(const utl::StringU8& word) : Word(word, /*color*/ 0, /* background_color*/ 0) {}

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
utl::StringU8 Paragraph::textFromCard(const Card& card) {
    utl::StringU8 text;
    if (const DialogueCard* dlgCard = dynamic_cast<const DialogueCard*>(&card)) {
        const std::string tbOpen = "<tr>";
        const std::string tbClose = "</tr>";
        const std::string open = "<td style=\"padding:10px 15px 10px 15px;\">";
        const std::string close = "</td>";
        for (const auto& dialogue : dlgCard->dialogue) {
            text.push_back({tbOpen, true, 0});
            text.push_back({open, true, 1});
            text.append(dialogue.speaker);
            text.push_back({close, true, 0});
            text.push_back({open, true, 1});
            text.append(dialogue.text);
            text.push_back({close, true, 0});
            text.push_back({tbClose, true, 0});
        }
    }
    if (const TextCard* textCard = dynamic_cast<const TextCard*>(&card)) {
        text = textCard->text;
    }

    return text;
}

Paragraph::Paragraph(const Card& _card, const std::shared_ptr<ZH_Dictionary>& _zh_dictionary)
    : zh_dictionary(_zh_dictionary) {
    auto card = std::unique_ptr<Card>(_card.clone());
    utl::StringU8 text = textFromCard(*card);

    std::cout << "Text: " << text << "\n";
    zh_annotator = std::make_unique<ZH_Annotator>(text, zh_dictionary);
    ranges::transform(zh_annotator->Items(),
                      std::back_inserter(*this),
                      [](const ZH_Annotator::Item& item) -> markup::Word {
                          //   std::cout << item.text << " : " << item.text.length() << "\n";
                          //   if (not item.dicItemVec.empty())
                          //       return {.word = item.text, .color = 0, .backGroundColor = 0x010101};
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

auto Paragraph::wordFromPosition(int pos) const -> const ZH_Annotator::ZH_dicItemVec {
    if (!zh_annotator)
        return {};

    const std::size_t index = getWordIndex(pos);
    if (index >= zh_annotator->Items().size())
        return {};

    const ZH_Annotator::Item& item = zh_annotator->Items().at(index);
    return item.dicItemVec;
}

void Paragraph::setupVocables(std::vector<ZH_Dictionary::Item>&& _vocables) {
    vocables = std::move(_vocables);

    ranges::sort(vocables, [this](const auto& a, const auto& b) {
        const auto a_it = ranges::find_if(
            zh_annotator->Items(), [&a](const auto& item) { return std::string(item.text) == a.key; });
        const auto b_it = ranges::find_if(
            zh_annotator->Items(), [&b](const auto& item) { return std::string(item.text) == b.key; });
        return a_it < b_it;
    });

    // const std::array colors = {0xee82ee, 0xff4500, 0x3cb371, 0x00ffff, 0xffff00, 0x7b68ee, 0xdc143c,
    //                            0xf08080, 0xbdb76b, 0xd8bfd8, 0x008b8b, 0x00bfff, 0x00fa9a, 0xff00ff,
    //                            0xd2691e, 0x32cd32, 0x9400d3, 0xdaa520, 0x556b2f, 0xb03060, 0x483d8b,
    //                            0x808080, 0xadff2f, 0x7f007f, 0x1e90ff, 0xff1493};
    const std::array colors = {0xe6194B,
                               0x3cb44b,
                               0xffe119,
                               0x4363d8,
                               0xf58231,
                               0x911eb4,
                               0x42d4f4,
                               0xf032e6,
                               0xbfef45,
                               0xfabed4,
                               0x469990};
    assert(zh_annotator->Items().size() == words.size());

    // clang-format off
    for (uint colorIndex = 0; colorIndex < vocables.size(); colorIndex++) {
        const ZH_Dictionary::Item& voc = vocables[colorIndex];

        for (boost::tuple<Word&, const ZH_Annotator::Item&> p : boost::combine(words,
                                                                               zh_annotator->Items())) {
            if (std::string(p.get<1>().text) == voc.key) {
                p.get<0>().setColor(colors[colorIndex % colors.size()]);
                break;
            }
        }
    }  // clang-format on

    vocableString = std::accumulate(
        vocables.begin(),
        vocables.end(),
        std::string{},
        [&, colorIndex = 0](std::string&& a, const ZH_Dictionary::Item& b) mutable {
            uint32_t color = colors[colorIndex++ % colors.size()];
            return a += fmt::format(  // clang-format off
                    "<tr>>"
                        "<td style=\"padding:0 15px 0 15px;color:#{c:06x};\">{}</td>"
                        "<td style=\"padding:0 15px 0 15px;color:#{c:06x};\">{}</td>"
                        "<td style=\"color:#{c:06x};\">{}</td>"
                    "</tr>",  // clang-format on
                       b.key,
                       b.pronounciation,
                       b.meanings.at(0),
                       fmt::arg("c", color));
        });
}

auto Paragraph::getVocableString() const -> std::string { return vocableString; }

}  // namespace markup
