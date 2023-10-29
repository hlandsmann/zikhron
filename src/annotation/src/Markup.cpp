#include "Markup.h"

#include <CardDB.h>
#include <Ease.h>
#include <Word.h>
#include <ZH_Annotator.h>
#include <dictionary/ZH_Dictionary.h>
#include <fmt/format.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <utils/Memoizer.h>
#include <utils/StringU8.h>

#include <algorithm>
#include <array>
#include <boost/range/combine.hpp>
#include <boost/tuple/tuple.hpp>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <sys/types.h>

namespace ranges = std::ranges;

namespace markup {

auto Paragraph::calculate_positions(size_t (Word::*len)() const) const -> std::vector<int>
{
    std::vector<int> result;
    result.push_back(0);
    ranges::transform(words,
                      std::back_inserter(result),
                      [absolutePosition = size_t(0), len](const Word& word) mutable -> size_t {
                          absolutePosition += std::invoke(len, word);
                          return absolutePosition;
                      });
    return result;
}

Paragraph::Paragraph(std::shared_ptr<Card> _card)
    : card(std::move(_card))
{
    utl::StringU8 text = card->getText();

    const ZH_Annotator& zh_annotator = card->getAnnotator();
    ranges::transform(zh_annotator.Items(),
                      std::back_inserter(words),
                      [](const ZH_Annotator::Item& item) -> markup::Word { return item.text; });

    utf8Positions = calculate_positions(&Word::vLength);
    bytePositions = calculate_positions(&Word::byteLength);

    fragments.clear();
    if (const auto* dlgCard = dynamic_cast<const DialogueCard*>(card.get())) {
        std::vector<size_t> fragmentPositions;
        size_t fragmentPos = 0;
        for (const auto& dialogue : dlgCard->dialogue) {
            fragmentPos += utl::StringU8(dialogue.speaker).length();
            fragmentPositions.push_back(fragmentPos);
            fragmentPos++;
            fragmentPos += utl::StringU8(dialogue.text).length();
            fragmentPositions.push_back(fragmentPos);
            fragmentPos++;
        }

        size_t firstWordIndex = 0;
        size_t wordIndex = 0;
        size_t fragmentIndex = 0;
        size_t currentLength = 0;
        for (const auto& item : zh_annotator.Items()) {
            currentLength += utl::StringU8(item.text).length();
            wordIndex++;

            if (fragmentIndex >= fragmentPositions.size()) {
                break;
            }
            if (currentLength == fragmentPositions[fragmentIndex]) {
                fragments.emplace_back(words.begin() + firstWordIndex, words.begin() + wordIndex);
                firstWordIndex = wordIndex + 1;
                fragmentIndex++;
            }
        }
    } else {
        fragments.emplace_back(words.begin(), words.end());
    }
    for (const auto& w : fragments) {
        auto nw = std::accumulate(w.begin(), w.end(), std::string{}, utl::stringPlusT<Word>);
    }
}

Paragraph::Paragraph(std::shared_ptr<Card> card_in, std::vector<VocableId>&& vocableIds_in)
    : Paragraph(std::move(card_in))
{
    vocableIds = std::move(vocableIds_in);
}

auto Paragraph::get() const -> std::string
{
    return std::accumulate(words.begin(), words.end(), std::string{}, utl::stringPlusT<Word>);
}

auto Paragraph::getFragments() const -> std::vector<std::string>
{
    std::vector<std::string> result;
    ranges::transform(fragments, std::back_inserter(result), [](const std::span<const Word>& fragment) {
        return std::accumulate(fragment.begin(), fragment.end(), std::string{}, utl::stringPlusT<Word>);
    });
    return result;
}

auto Paragraph::getWordStartPosition(int pos, const std::vector<int>& positions) const -> int
{
    std::size_t index = getWordIndex(pos, positions);
    if (index >= positions.size()) {
        return -1;
    }
    return positions[index];
}

auto Paragraph::getWordIndex(int pos, const std::vector<int>& positions) const -> std::size_t
{
    auto posIt = std::adjacent_find(positions.begin(), positions.end(), [pos](int posA, int posB) {
        return posA <= pos && posB > pos;
    });
    if (posIt == positions.end()) {
        if (pos > positions.back() || pos < 0) {
            return std::numeric_limits<std::size_t>::max();
        }
        posIt = std::prev(positions.end());
    }
    return std::max<size_t>(0, std::distance(positions.begin(), posIt));
}

auto Paragraph::fragmentStartPos(size_t fragment, const std::vector<int>& positions) const -> int
{
    if (fragment >= fragments.size()) {
        throw std::out_of_range(
                std::format("Only {} fragments, fragment #{} requested", fragments.size(), fragment));
    }
    ptrdiff_t dist = std::distance(std::span<const Word>(words).begin(), fragments[fragment].begin());

    int result = positions[dist];
    return result;
}

void Paragraph::updateAnnotationColoring()
{
    const auto& zh_annotator = card->getAnnotator();
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
                    return val + std::max(0, (itemSizes.empty() ? 0 : *ranges::max_element(itemSizes)) - (val - n++));
                });
    });

    auto numberOfCombinations = utl::Single_memoinize(+[](decltype(chunks.begin()) chunkIt) -> size_t {
        return ZH_Annotator::get_combinations(*chunkIt).size();
    });

    size_t currentPos = 0;
    auto chunkIt = chunks.begin();
    for (const auto& [word, item] : boost::combine(words, items)) {
        if (word.isMarkup() || item.dicItemVec.empty()) {
            currentPos += word.byteLength();
            annotationChunk.posBegin = currentPos;
            continue;
        }
        while (chunkIt != chunks.end() && (chunkIt->empty() || chunkIt->front().empty())) {
            chunkIt++;
        }

        if (chunkIt == chunks.end()) {
            break;
        }

        currentPos += word.byteLength();

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

void Paragraph::highlightWordAtPosition(int pos, const std::vector<int>& positions)
{
    if (positions.empty()) {
        return;
    }
    size_t index = getWordIndex(pos, positions);

    if (index >= words.size()) {
        return;
    }
    if (words[index].isMarkup()) {
        return;
    }

    preChanges.push({.index = index, .word{words[index]}});

    const ZH_Annotator::ZH_dicItemVec clickedItem = wordFromPosition(pos, positions);
    if (clickedItem.empty()) {
        return;
    }
    Word& word = words[index];
    word.setBackgroundColor(0x227722);
    word.setColor(0xFFFFFF);
}

auto Paragraph::getAnnotationChunkFromPosition(size_t pos)
        -> std::optional<std::reference_wrapper<AnnotationChunk>>
{
    auto annoIt = ranges::find_if(annotationChunks, [pos](const AnnotationChunk& annotationChunk) {
        return annotationChunk.posBegin <= pos && pos < annotationChunk.posEnd;
    });
    if (annoIt == annotationChunks.end()) {
        return {};
    }
    return *annoIt;
}

void Paragraph::highlightAnnotationAtPosition(int pos)
{
    auto annotation = getAnnotationChunkFromPosition(pos);
    if (not annotation.has_value()) {
        return;
    }
    AnnotationChunk& annotationChunk = annotation.value().get();
    const auto& colors = markingColors_blue;
    int cw = 0;
    for (auto& word : annotationChunk.words) {
        preChanges.push(
                {.index = static_cast<size_t>(std::distance(words.data(), &word.get())), .word = word});
        word.get().setBackgroundColor(colors[cw++ % 2]);
    }
}

auto Paragraph::getAnnotationPossibilities(int pos) -> AnnotationPossibilities
{
    auto annotation = getAnnotationChunkFromPosition(pos);
    if (not annotation.has_value()) {
        return {};
    }
    AnnotationChunk& annotationChunk = annotation.value().get();
    const auto& markingColors = std::distance(&(*std::begin(annotationChunks)), &annotationChunk) % 2 == 0
                                        ? markingColors_red
                                        : markingColors_green;

    std::vector<std::string> marked;
    std::vector<std::string> unmarked;

    const std::vector<utl::CharU8>& items = annotationChunk.characters;

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

    return {.activeChoice = std::accumulate(annotationChunk.words.begin(),
                                            annotationChunk.words.end(),
                                            std::string{},
                                            utl::stringPlusT<Word>),
            .unmarked = unmarked,
            .marked = marked,
            .pos = annotationChunk.posBegin,
            .combinations = combinations,
            .characters = annotationChunk.characters};
}

void Paragraph::undoChange()
{
    if (preChanges.empty()) {
        return;
    }
    while (not preChanges.empty()) {
        WordState preChange = preChanges.top();
        preChanges.pop();
        words[preChange.index] = std::move(preChange.word);
    }
}

auto Paragraph::wordFromPosition(int pos, const std::vector<int>& positions) const
        -> const ZH_Annotator::ZH_dicItemVec
{
    const auto& zh_annotator = card->getAnnotator();

    const std::size_t index = getWordIndex(pos, positions);
    if (index >= zh_annotator.Items().size()) {
        return {};
    }

    const ZH_Annotator::Item& item = zh_annotator.Items().at(index);
    return item.dicItemVec;
}

auto Paragraph::getVocableChoiceFromPosition(int pos, const std::vector<int>& positions) const
        -> ZH_Dictionary::Entry
{
    const auto& zh_annotator = card->getAnnotator();
    const std::size_t index = getWordIndex(pos, positions);
    if (index >= zh_annotator.Items().size()) {
        return {};
    }
    size_t indexVocableChoice = std::accumulate(
            zh_annotator.Items().begin(),
            zh_annotator.Items().begin() + index,
            size_t{},
            [](size_t start, const ZH_Annotator::Item& annotatorItem) {
                if (!annotatorItem.dicItemVec.empty()) {
                    return start + 1;
                }
                return start;
            });
    assert(indexVocableChoice < vocableIds.size());
    size_t vocId = vocableIds[indexVocableChoice];
    const ZH_Dictionary& zh_dictionary = *card->getAnnotator().Dictionary();

    return zh_dictionary.EntryFromPosition(vocId, CharacterSetType::Simplified);
}

auto Paragraph::getVocables() const -> const std::vector<vocable_pronounciation_meaning_t>&
{
    return vocables;
}

void Paragraph::setupVocables(const std::map<VocableId, Ease>& ease)
{
    const auto& zh_annotator = card->getAnnotator();

    const ZH_Dictionary& zh_dictionary = *card->getAnnotator().Dictionary();
    activeVocables.clear();
    ranges::copy(ease | std::views::keys, std::back_inserter(activeVocables));
    ranges::sort(activeVocables, std::less{}, [&](const auto& vocId) {
        return ranges::find_if(zh_annotator.Items(), [&](const auto& item) -> bool {
            const auto& key = zh_dictionary.EntryFromPosition(vocId, CharacterSetType::Simplified).key;
            return std::string(item.text) == key;
        });
    });

    const std::array<uint32_t, 11> colors = {0xffe119, 0x3cd44b, 0x42d4f4,
                                             0xf58231, 0xf032e6, 0xfabed4,
                                             0x913ec4, 0xff294B, 0x4363ff,
                                             0xbfef45, 0x469990};
    assert(zh_annotator.Items().size() == words.size());

    const auto& items = zh_annotator.Items();
    for (uint colorIndex = 0; colorIndex < activeVocables.size(); colorIndex++) {
        uint vocId = activeVocables[colorIndex];
        const ZH_Dictionary::Entry& voc = zh_dictionary.EntryFromPosition(vocId,
                                                                          CharacterSetType::Simplified);

        for (boost::tuple<Word&, const ZH_Annotator::Item&> p : boost::combine(words, items)) {
            auto& word = p.get<0>();
            const auto& item = p.get<1>();

            if (std::string(item.text) == voc.key) {
                word.setColor(colors[colorIndex % colors.size()]);
                break;
            }
        }
    }
    ranges::transform(
            activeVocables,
            std::back_inserter(vocables),
            [&, colorIndex = size_t{0}](uint vocId) mutable -> vocable_pronounciation_meaning_t {
                const ZH_Dictionary::Entry& zhEntry = zh_dictionary.EntryFromPosition(
                        vocId, CharacterSetType::Simplified);
                uint32_t color = colors[colorIndex++ % colors.size()];
                std::string style = std::format(" color=\"#{:06x}\"", color);

                std::string vocable = std::format("<span{}>{}</span>", style, zhEntry.key);
                std::string pronounciation = std::format("<span{}>{}</span>", style, zhEntry.pronounciation);
                std::string meaning = std::format("<span{}>{}</span>", style, zhEntry.meanings.at(0));
                return {vocable, pronounciation, meaning};
            });
}

auto Paragraph::getRelativeOrderedEaseList(const std::map<VocableId, Ease>& ease_in) const
        -> std::vector<std::pair<VocableId, Ease>>
{
    std::vector<std::pair<VocableId, Ease>> ease_out;

    for (VocableId vocId : vocableIds) {
        const auto itEase_in = ease_in.find(vocId);
        const auto itEase_out = ranges::find(ease_out, vocId, &std::pair<VocableId, Ease>::first);
        if (itEase_in != ease_in.end() && itEase_out == ease_out.end()) {
            ease_out.emplace_back(vocId, itEase_in->second);
        }
    }
    return ease_out;
};

auto Paragraph::getRestoredOrderOfEaseList(const std::vector<Ease>& ease_in) const
        -> std::map<VocableId, Ease>
{
    assert(ease_in.size() == activeVocables.size());

    std::map<VocableId, Ease> ease_out;
    for (const auto& [vocId, ease] : boost::combine(activeVocables, ease_in)) {
        ease_out.insert({vocId, ease});
    }

    return ease_out;
}

} // namespace markup
