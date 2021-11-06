#include "Markup.h"
#include <fmt/format.h>
#include <utils/Memoizer.h>
#include <algorithm>
#include <boost/range/combine.hpp>
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

auto Paragraph::textFromCard(const Card& card) -> utl::StringU8 {
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

Paragraph::Paragraph(std::unique_ptr<Card> _card) : card(std::move(_card)) {
    utl::StringU8 text = textFromCard(*card);

    assert(card->zh_annotator.has_value());
    const ZH_Annotator& zh_annotator = card->zh_annotator.value();
    ranges::transform(zh_annotator.Items(),
                      std::back_inserter(words),
                      [](const ZH_Annotator::Item& item) -> markup::Word { return item.text; });

    positions.push_back(0);
    ranges::transform(
        words, std::back_inserter(positions), [absolutePosition = 0](const Word& word) mutable -> int {
            absolutePosition += word.vLength();
            return absolutePosition;
        });
}

auto Paragraph::get() const -> std::string {
    return std::accumulate(words.begin(), words.end(), std::string{}, utl::stringPlusT<Word>);
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

void Paragraph::updateAnnotationColoring() {
    assert(card->zh_annotator.has_value());

    const auto& zh_annotator = card->zh_annotator.value();
    const auto& chunks = zh_annotator.Chunks();
    const auto& items = zh_annotator.Items();

    if (words.empty())
        return;
    assert(words.size() == items.size());

    annotationChunks.clear();

    size_t size = 0;
    AnnotationChunk annotationChunk = {};

    auto lengthOfChunk = utl::Single_memoinize(+[](decltype(chunks.begin()) chunkIt) -> size_t {
        return std::accumulate(
            chunkIt->begin(),
            chunkIt->end(),
            0,
            [n = 0](int val, const std::vector<int>& itemSizes) mutable -> int {
                return val +
                       std::max(0,
                                (itemSizes.empty() ? 0 : *ranges::max_element(itemSizes)) - (val - n++));
            });
    });

    auto numberOfCombinations = utl::Single_memoinize(+[](decltype(chunks.begin()) chunkIt) -> size_t {
        return ZH_Annotator::get_combinations(*chunkIt).size();
    });

    int currentPos = 0;
    auto chunkIt = chunks.begin();
    for (const auto& [word, item] : boost::combine(words, items)) {
        if (word.isMarkup() || item.dicItemVec.empty()) {
            currentPos += word.vLength();
            annotationChunk.posBegin = currentPos;
            continue;
        }
        while (chunkIt != chunks.end() && (chunkIt->empty() || chunkIt->front().empty()))
            chunkIt++;

        if (chunkIt == chunks.end())
            break;

        currentPos += word.vLength();

        if (numberOfCombinations.evaluate(chunkIt) < 2) {
            chunkIt++;
            annotationChunk.posBegin = currentPos;
            continue;
        }

        annotationChunk.words.push_back(word);
        std::copy(item.text.cbegin(), item.text.cend(), std::back_inserter(annotationChunk.characters));
        size += item.text.length();

        if (size == lengthOfChunk.evaluate(chunkIt)) {
            annotationChunk.chunk = *chunkIt;
            annotationChunk.posEnd = currentPos;
            annotationChunks.push_back(std::move(annotationChunk));
            annotationChunk.clear();
            annotationChunk.posBegin = currentPos;
            size = 0;

            chunkIt++;
        }
    }
    int counter = 0;

    for (auto& ac : annotationChunks) {
        const auto& colors = counter++ % 2 == 0 ? markingColors_red : markingColors_green;
        int cw = 0;
        for (auto& word : ac.words) {
            word.get().setBackgroundColor(colors[cw++ % 2]);
        }
    }
}

void Paragraph::highlightWordAtPosition(int pos) {
    if (positions.empty())
        return;
    size_t index = getWordIndex(pos);

    if (index >= words.size())
        return;
    if (words[index].isMarkup())
        return;

    preChanges.push({.index = index, .word{words[index]}});

    const ZH_Annotator::ZH_dicItemVec clickedItem = wordFromPosition(pos);
    if (clickedItem.empty())
        return;
    Word& word = words[index];
    word.setBackgroundColor(0x227722);
    word.setColor(0xFFFFFF);
}

auto Paragraph::getAnnotationChunkFromPosition(int pos)
    -> std::optional<std::reference_wrapper<AnnotationChunk>> {
    auto annoIt = ranges::find_if(annotationChunks, [pos](const AnnotationChunk& annotationChunk) {
        return annotationChunk.posBegin <= pos && pos < annotationChunk.posEnd;
    });
    if (annoIt == annotationChunks.end())
        return {};
    return *annoIt;
}

void Paragraph::highlightAnnotationAtPosition(int pos) {
    auto annotation = getAnnotationChunkFromPosition(pos);
    if (not annotation.has_value())
        return;
    AnnotationChunk& annotationChunk = annotation.value().get();
    const auto& colors = markingColors_blue;
    int cw = 0;
    for (auto& word : annotationChunk.words) {
        preChanges.push(
            {.index = static_cast<size_t>(std::distance(words.data(), &word.get())), .word = word});
        word.get().setBackgroundColor(colors[cw++ % 2]);
    }
}

auto Paragraph::getAnnotationPossibilities(int pos) -> AnnotationPossibilities {
    auto annotation = getAnnotationChunkFromPosition(pos);
    if (not annotation.has_value())
        return {};
    AnnotationChunk& annotationChunk = annotation.value().get();
    const auto& markingColors = std::distance(&(*std::begin(annotationChunks)), &annotationChunk) % 2 ==
                                        0
                                    ? markingColors_red
                                    : markingColors_green;

    std::vector<std::string> marked;
    std::vector<std::string> unmarked;

    const std::vector<utl::ItemU8>& items = annotationChunk.characters;

    const auto combinations = ZH_Annotator::get_combinations(annotationChunk.chunk);
    for (const auto& combination : combinations) {
        // fmt::print("{}\n", fmt::join(combination, ","));
        int currentPos = 0;
        std::string markedCombination;
        std::string unmarkedCombination;
        int cw = 0;
        for (int l : combination) {
            auto word = Word(utl::StringU8(std::span(items.begin() + currentPos, l)));
            currentPos += l;
            word.setColor(0x00FFFFFF);
            word.setBackgroundColor(markingColors[cw % 2]);
            markedCombination += std::string(word);
            word.setBackgroundColor(markingColors_blue[cw % 2]);
            unmarkedCombination += std::string(word);
            cw++;
        }

        marked.push_back(std::move(markedCombination));
        unmarked.push_back(std::move(unmarkedCombination));
    }

    return {unmarked, marked, annotationChunk.posBegin, combinations, annotationChunk.characters};
}

void Paragraph::undoChange() {
    if (preChanges.empty())
        return;
    while (not preChanges.empty()) {
        WordState preChange = preChanges.top();
        preChanges.pop();
        words[preChange.index] = std::move(preChange.word);
    }
}

auto Paragraph::wordFromPosition(int pos) const -> const ZH_Annotator::ZH_dicItemVec {
    assert(card->zh_annotator.has_value());
    const auto& zh_annotator = card->zh_annotator.value();

    const std::size_t index = getWordIndex(pos);
    if (index >= zh_annotator.Items().size())
        return {};

    const ZH_Annotator::Item& item = zh_annotator.Items().at(index);
    return item.dicItemVec;
}

void Paragraph::setupVocables(std::vector<std::pair<ZH_Dictionary::Item, uint>>&& _vocables_id) {
    vocables_id = std::move(_vocables_id);
    assert(card->zh_annotator.has_value());
    const auto& zh_annotator = card->zh_annotator.value();

    ranges::sort(vocables_id, std::less{}, [&](const auto& a) {
        return ranges::find_if(zh_annotator.Items(),
                               [&a](const auto& item) { return std::string(item.text) == a.first.key; });
    });

    // clang-format off
    const std::array colors = {0xfabed4, 0xffe119, 0xff294B, 0x3cd44b, 0x42d4f4, 0xf58231,
                               0xf032e6, 0x913ec4, 0x4363ff, 0xbfef45, 0x469990};
    // clang-format on
    assert(zh_annotator.Items().size() == words.size());

    const auto& items = zh_annotator.Items();
    for (uint colorIndex = 0; colorIndex < vocables_id.size(); colorIndex++) {
        const ZH_Dictionary::Item& voc = vocables_id[colorIndex].first;

        for (boost::tuple<Word&, const ZH_Annotator::Item&> p : boost::combine(words, items)) {
            auto& word = p.get<0>();
            const auto& item = p.get<1>();

            if (std::string(item.text) == voc.key) {
                word.setColor(colors[colorIndex % colors.size()]);
                break;
            }
        }
    }

    vocableString = std::accumulate(
        vocables_id.begin(),
        vocables_id.end(),
        std::string{},
        [&, colorIndex = 0](std::string&& a, const auto& inItem) mutable {
            const ZH_Dictionary::Item& b = inItem.first;
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
    vocablePositions.clear();
    ranges::transform(
        vocables_id,
        std::back_inserter(vocablePositions),
        [n = 1](const ZH_Dictionary::Item& voc) mutable {
            int temp = n;  // clang-format off
            n += utl::StringU8(voc.key).length() + 1 +
                 utl::StringU8(voc.pronounciation).length() + 1 +
                 utl::StringU8(voc.meanings.at(0)).length() + 1;
            return temp;  // clang-format on
        },
        [](const auto& in) { return in.first; });
}

auto Paragraph::getVocableString() const -> std::string { return vocableString; }
auto Paragraph::getVocablePositions() const -> const std::vector<int>& { return vocablePositions; };
auto Paragraph::getRelativeOrderedEaseList(const std::map<uint, Ease>& ease_in) const
    -> std::vector<std::pair<uint, Ease>> {
    std::vector<std::pair<uint, Ease>> ease_out;
    ranges::transform(
        vocables_id,
        std::back_inserter(ease_out),
        [&ease_in](const auto& id) -> std::pair<uint, Ease> {
            return {id, ease_in.at(id)};
        },
        [](const auto& m) { return m.second; });
    return ease_out;
};

auto Paragraph::getRestoredOrderOfEaseList(const std::vector<Ease>& ease_in) const
    -> std::map<uint, Ease> {
    assert(ease_in.size() == vocables_id.size());

    std::map<uint, Ease> ease_out;
    ranges::transform(vocables_id,
                      ease_in,
                      std::inserter(ease_out, ease_out.begin()),
                      [](const auto& voc_id, const auto& ease) -> std::pair<uint, Ease> {
                          return {voc_id.second, ease};
                      });
    return ease_out;
}

}  // namespace markup
