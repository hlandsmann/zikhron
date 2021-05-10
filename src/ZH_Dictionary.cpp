#include "ZH_Dictionary.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <tuple>

namespace ranges = std::ranges;

namespace {
auto splitOnce(const std::string_view& str, const char delim)
    -> std::pair<std::string_view, std::string_view> {
    std::size_t found = str.find(delim);
    if (found == std::string_view::npos) {
        return {str, std::string_view()};
    }
    return {str.substr(0, found), str.substr(found + 1)};
}

auto extractSubstr(const std::string_view& str, const char delimBegin, const char delimEnd)
    -> std::pair<std::string_view, std::string_view> {
    std::size_t foundBegin = str.find(delimBegin);
    std::size_t foundEnd = str.find(delimEnd);
    if (foundBegin == std::string_view::npos || foundEnd == std::string_view::npos ||
        foundBegin > foundEnd) {
        return {std::string_view(), std::string_view()};
    }
    return {str.substr(foundBegin + 1, foundEnd - 1 - foundBegin), str.substr(foundEnd + 1)};
}

auto transformPronounciation(const std::string_view& pronounciation) -> std::string {
    static const std::array toneVowels = {
        std::array<std::string, 5>({"ā", "á", "ǎ", "à", "a"}),
        std::array<std::string, 5>({"ē", "é", "ě", "è", "e"}),
        std::array<std::string, 5>({"ī", "í", "ǐ", "ì", "i"}),
        std::array<std::string, 5>({"ō", "ó", "ǒ", "ò", "o"}),
        std::array<std::string, 5>({"ǖ", "ǘ", "ǚ", "ǜ", "ü"}),  // 3 times ü
        std::array<std::string, 5>({"ǖ", "ǘ", "ǚ", "ǜ", "ü"}),
        std::array<std::string, 5>({"ǖ", "ǘ", "ǚ", "ǜ", "ü"}),
        std::array<std::string, 5>({"ū", "ú", "ǔ", "ù", "u"}),
        std::array<std::string, 5>({"Ā", "Á", "Ǎ", "À", "A"}),
        std::array<std::string, 5>({"Ē", "É", "Ě", "È", "E"}),
        std::array<std::string, 5>({"Ī", "Í", "Ǐ", "Ì", "I"}),
        std::array<std::string, 5>({"Ō", "Ó", "Ǒ", "Ò", "O"}),
        std::array<std::string, 5>({"Ǖ", "Ǘ", "Ǚ", "Ǜ", "Ü"}),  // Again ...
        std::array<std::string, 5>({"Ǖ", "Ǘ", "Ǚ", "Ǜ", "Ü"}),  // .. 3 times ü
        std::array<std::string, 5>({"Ǖ", "Ǘ", "Ǚ", "Ǜ", "Ü"}),
        std::array<std::string, 5>({"Ǖ", "Ǘ", "Ǚ", "Ǜ", "Ü"}),
        std::array<std::string, 5>({"Ū", "Ú", "Ǔ", "Ù", "U"})};

    // clang-format off
    static const std::array<std::string, 16> vowels = {"a", "e", "i", "o", "u:", "v", "ü", "u",
                                                       "A", "E", "I", "O", "U:", "V", "Ü", "U"};
    // clang-format on
    constexpr std::array<char, 5> tones = {'1', '2', '3', '4', '5'};

    std::string finalResult;
    finalResult.reserve(pronounciation.size());
    int startIndex = 0;

    auto [syllable, rest] = splitOnce(pronounciation, ' ');
    while (not syllable.empty()) {
        finalResult += syllable.substr(0, syllable.length() - 1);

        const auto toneIt = std::find(tones.begin(), tones.end(), syllable.back());
        if (toneIt != tones.end() && syllable.size() > 1) {
            const int toneIndex = std::distance(tones.begin(), toneIt);

            for (const auto& vowel : vowels) {
                const auto vowelIndex = finalResult.find(vowel, startIndex);
                if (vowelIndex == std::string::npos)
                    continue;

                const int i = std::distance(vowels.begin(), &vowel);
                finalResult.replace(vowelIndex, vowel.length(), toneVowels[i][toneIndex]);
                break;
            }
        } else {
            finalResult.push_back(syllable.back());
        }

        std::tie(syllable, rest) = splitOnce(rest, ' ');
        if (not syllable.empty())
            finalResult.push_back(' ');
        startIndex = finalResult.size();
    }

    return finalResult;
}

auto transformMeaning(const std::string_view& meaning_raw) -> std::string {
    size_t delimPos = meaning_raw.find('[');
    if (delimPos == std::string::npos)
        return std::string(meaning_raw);

    std::string finalResult;
    finalResult.reserve(meaning_raw.size());

    std::string_view rest = meaning_raw;
    std::string_view pron_raw;
    do {
        std::string_view intermediate = rest.substr(0, delimPos);
        finalResult += intermediate;

        std::tie(pron_raw, rest) = extractSubstr(rest, '[', ']');
        if (pron_raw.empty())
            break;

        finalResult += '[' + transformPronounciation(pron_raw) + ']';
        delimPos = rest.find('[');
    } while (not rest.empty());

    return std::string(meaning_raw);
}

struct DictionaryItem_raw {
    std::string_view traditional;
    std::string_view simplified;
    std::string pronounciation;

    std::vector<std::string> meanings = {};
};

auto parseLine(const std::string_view& line) -> DictionaryItem_raw {
    std::string_view traditional, simplified, pron_raw, rest;
    std::tie(traditional, rest) = splitOnce(line, ' ');
    std::tie(simplified, rest) = splitOnce(rest, ' ');
    std::tie(pron_raw, rest) = extractSubstr(rest, '[', ']');

    DictionaryItem_raw dicItem = {.traditional = traditional,
                                  .simplified = simplified,
                                  .pronounciation = transformPronounciation(pron_raw)};

    std::tie(std::ignore, rest) = splitOnce(rest, '/');
    std::string_view meaning;

    std::tie(meaning, rest) = splitOnce(rest, '/');
    while (not meaning.empty()) {
        dicItem.meanings.push_back(transformMeaning(meaning));
        std::tie(meaning, rest) = splitOnce(rest, '/');
    }

    return dicItem;
}

}  // namespace

ZH_Dictionary::ZH_Dictionary(const std::string& filename) {
    std::ifstream dictFile(filename);
    if (!dictFile)
        throw std::runtime_error("Could not open dictionary file: '" + filename + "'");

    unsigned position = 0;
    for (std::string line; getline(dictFile, line);) {
        // ToDo: filter BOM which is created by windows notepad, Bytes: [0xEF, 0xBB, 0xBF] at beginning
        // of file (case with HanDeDict.u8)
        if (line.empty() || line.at(0) == '#')
            continue;

        auto dicItem = parseLine(line);
        traditional.push_back({.key = std::string(dicItem.traditional), .pos = position});
        simplified.push_back({.key = std::string(dicItem.simplified), .pos = position});
        pronounciation.push_back(std::move(dicItem.pronounciation));
        meanings.push_back(std::move(dicItem.meanings));

        position++;
    }

    if (not ranges::is_sorted(simplified, std::ranges::less{}, &ZH_Dictionary::Key::key))
        ranges::sort(simplified, std::ranges::less{}, &ZH_Dictionary::Key::key);
    if (not ranges::is_sorted(traditional, std::ranges::less{}, &ZH_Dictionary::Key::key))
        ranges::sort(traditional, std::ranges::less{}, &ZH_Dictionary::Key::key);

    position_to_simplified.resize(traditional.size());
    position_to_traditional.resize(traditional.size());

    for (unsigned i = 0; i < simplified.size(); i++)
        position_to_simplified[simplified[i].pos] = i;

    for (unsigned i = 0; i < traditional.size(); i++)
        position_to_traditional[traditional[i].pos] = i;
}

auto ZH_Dictionary::Lower_bound(const std::string& key, const std::span<const Key>& characterSet)
    -> std::span<const Key> {
    return Lower_bound(std::string_view(key), characterSet);
}

auto ZH_Dictionary::Lower_bound(const std::string_view& key, const std::span<const Key>& characterSet)
    -> std::span<const Key> {
    return {ranges::lower_bound(characterSet, key, ranges::less{}, &Key::key), characterSet.end()};
}

auto ZH_Dictionary::Upper_bound(const std::string& key, const std::span<const Key>& characterSet)
    -> std::span<const Key> {
    return Upper_bound(std::string_view(key), characterSet);
}

auto ZH_Dictionary::Upper_bound(const std::string_view& key, const std::span<const Key>& characterSet)
    -> std::span<const Key> {
    return {characterSet.begin(),
            ranges::upper_bound(characterSet, key, ranges::less{}, [&key](const Key& k) {
                return k.key.substr(0, key.length());
            })};
}

auto ZH_Dictionary::CharacterSetFromKeySpan(const std::span<const Key>& keys) const -> CharacterSet {
    auto sameSpan = [](const std::span<const Key>& a, const std::span<const Key>& b) -> bool {
        return (a.begin() == b.begin()) && (a.end() == b.end());
    };
    if (sameSpan(keys, Simplified()))
        return CharacterSet::Simplified;
    if (sameSpan(keys, Traditional()))
        return CharacterSet::Traditional;
    throw std::invalid_argument("Invalid choice other than traditional / simplified!");
}

auto ZH_Dictionary::ItemFromPosition(size_t pos, const std::span<const Key>& keys) const -> Item {
    switch (CharacterSetFromKeySpan(keys)) {
    case CharacterSet::Simplified: return ItemFromPosition(pos, CharacterSet::Simplified);
    case CharacterSet::Traditional: return ItemFromPosition(pos, CharacterSet::Traditional);
    }
    __builtin_unreachable();
}

auto ZH_Dictionary::ItemFromPosition(size_t pos, CharacterSet characterSet) const -> Item {
    const auto& pos_to_characterSet = (characterSet == CharacterSet::Simplified)
                                          ? position_to_simplified
                                          : position_to_traditional;
    const auto& keys = (characterSet == CharacterSet::Simplified) ? Simplified() : Traditional();
    return {
        .key = keys[pos_to_characterSet[pos]].key,
        .pronounciation = pronounciation.at(pos),
        .meanings = meanings.at(pos),
    };
}

auto ZH_Dictionary::Item::operator<=>(const Item& other) const -> std::weak_ordering {
    if (const auto cmp = key <=> other.key; cmp != 0)
        return cmp;
    if (const auto cmp = pronounciation <=> other.pronounciation; cmp != 0)
        return cmp;
    return meanings <=> other.meanings;
}
