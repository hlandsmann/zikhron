#include <DictionaryChi.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <utils/string_split.h>

#include <algorithm>
#include <array>
#include <compare>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace {

auto transformPronounciation(const std::string_view& pronounciation) -> std::string
{
    static const std::array toneVowels = {
            std::array<std::string, 5>({"ā", "á", "ǎ", "à", "a"}),
            std::array<std::string, 5>({"ē", "é", "ě", "è", "e"}),
            std::array<std::string, 5>({"ī", "í", "ǐ", "ì", "i"}),
            std::array<std::string, 5>({"ō", "ó", "ǒ", "ò", "o"}),
            std::array<std::string, 5>({"ǖ", "ǘ", "ǚ", "ǜ", "ü"}), // 3 times ü
            std::array<std::string, 5>({"ǖ", "ǘ", "ǚ", "ǜ", "ü"}),
            std::array<std::string, 5>({"ǖ", "ǘ", "ǚ", "ǜ", "ü"}),
            std::array<std::string, 5>({"ū", "ú", "ǔ", "ù", "u"}),
            std::array<std::string, 5>({"Ā", "Á", "Ǎ", "À", "A"}),
            std::array<std::string, 5>({"Ē", "É", "Ě", "È", "E"}),
            std::array<std::string, 5>({"Ī", "Í", "Ǐ", "Ì", "I"}),
            std::array<std::string, 5>({"Ō", "Ó", "Ǒ", "Ò", "O"}),
            std::array<std::string, 5>({"Ǖ", "Ǘ", "Ǚ", "Ǜ", "Ü"}), // Again ...
            std::array<std::string, 5>({"Ǖ", "Ǘ", "Ǚ", "Ǜ", "Ü"}), // .. 3 times ü
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
    size_t startIndex = 0;

    auto [syllable, rest] = utl::splitOnce(pronounciation, ' ');
    while (not syllable.empty()) {
        finalResult += syllable.substr(0, syllable.length() - 1);

        const auto* const toneIt = std::find(tones.begin(), tones.end(), syllable.back());
        if (toneIt != tones.end() && syllable.size() > 1) {
            const auto toneIndex = static_cast<size_t>(std::distance(tones.begin(), toneIt));

            for (const auto& vowel : vowels) {
                const auto vowelIndex = finalResult.find(vowel, startIndex);
                if (vowelIndex == std::string::npos) {
                    continue;
                }

                const auto i = static_cast<size_t>(std::distance(vowels.begin(), &vowel));
                finalResult.replace(vowelIndex, vowel.length(), toneVowels[i][toneIndex]);
                break;
            }
        } else {
            finalResult.push_back(syllable.back());
        }

        std::tie(syllable, rest) = utl::splitOnce(rest, ' ');
        if (not syllable.empty()) {
            finalResult.push_back(' ');
        }
        startIndex = finalResult.size();
    }

    return finalResult;
}

auto transformMeaning(const std::string_view& meaning_raw) -> std::string
{
    size_t delimPos = meaning_raw.find('[');
    if (delimPos == std::string::npos) {
        return std::string(meaning_raw);
    }

    std::string finalResult;
    finalResult.reserve(meaning_raw.size());

    std::string_view rest = meaning_raw;
    std::string_view pron_raw;
    do {
        std::string_view intermediate = rest.substr(0, delimPos);
        finalResult += intermediate;

        std::tie(pron_raw, rest) = utl::extractSubstr(rest, '[', ']');
        if (pron_raw.empty()) {
            break;
        }

        finalResult += '[' + transformPronounciation(pron_raw) + ']';
        delimPos = rest.find('[');
    } while (not rest.empty());

    return std::string(meaning_raw);
}

struct DictionaryItem_raw
{
    std::string_view traditional;
    std::string_view simplified;
    std::string pronounciation;

    std::vector<std::string> meanings = {};
};

auto parseLine(const std::string_view& line) -> DictionaryItem_raw
{
    std::string_view traditional;
    std::string_view simplified;
    std::string_view pron_raw;
    std::string_view rest;

    std::tie(traditional, rest) = utl::splitOnce(line, ' ');
    std::tie(simplified, rest) = utl::splitOnce(rest, ' ');
    std::tie(pron_raw, rest) = utl::extractSubstr(rest, '[', ']');

    DictionaryItem_raw dicItem = {.traditional = traditional,
                                  .simplified = simplified,
                                  .pronounciation = transformPronounciation(pron_raw)};

    std::tie(std::ignore, rest) = utl::splitOnce(rest, '/');
    std::string_view meaning;

    std::tie(meaning, rest) = utl::splitOnce(rest, '/');
    while (not meaning.empty()) {
        const auto transformed = transformMeaning(meaning);
        if (transformed.size() > 1) {
            dicItem.meanings.push_back(transformed);
        }
        std::tie(meaning, rest) = utl::splitOnce(rest, '/');
    }

    return dicItem;
}

} // namespace

namespace dictionary {
DictionaryChi::DictionaryChi(std::shared_ptr<zikhron::Config> config)
{
    const auto& filename = config->Dictionary() / s_fn_dictionary;
    std::ifstream dictFile(filename);
    if (!dictFile) {
        throw std::runtime_error("Could not open dictionary file: '" + filename.string() + "'");
    }

    unsigned position = 0;
    for (std::string line; getline(dictFile, line);) {
        // ToDo: filter BOM which is created by windows notepad, Bytes: [0xEF, 0xBB, 0xBF] at beginning
        // of file (case with HanDeDict.u8)
        if (line.empty() || line.at(0) == '#') {
            continue;
        }

        auto dicItem = parseLine(line);
        traditional.push_back({.key = std::string(dicItem.traditional), .pos = position});
        simplified.push_back({.key = std::string(dicItem.simplified), .pos = position});
        pronounciation.push_back(std::move(dicItem.pronounciation));
        meanings.push_back(std::move(dicItem.meanings));

        position++;
    }

    if (not ranges::is_sorted(simplified, std::ranges::less{}, &DictionaryChi::Key::key)) {
        ranges::sort(simplified, std::ranges::less{}, &DictionaryChi::Key::key);
    }
    if (not ranges::is_sorted(traditional, std::ranges::less{}, &DictionaryChi::Key::key)) {
        ranges::sort(traditional, std::ranges::less{}, &DictionaryChi::Key::key);
    }

    position_to_simplified.resize(traditional.size());
    position_to_traditional.resize(traditional.size());

    for (unsigned i = 0; i < simplified.size(); i++) {
        position_to_simplified[simplified[i].pos] = i;
    }

    for (unsigned i = 0; i < traditional.size(); i++) {
        position_to_traditional[traditional[i].pos] = i;
    }
}

auto DictionaryChi::Lower_bound(const std::string& key, const std::span<const Key>& characterSet)
        -> std::span<const Key>
{
    return Lower_bound(std::string_view(key), characterSet);
}

auto DictionaryChi::Lower_bound(const std::string_view& key, const std::span<const Key>& characterSet)
        -> std::span<const Key>
{
    return {ranges::lower_bound(characterSet, key, ranges::less{}, &Key::key), characterSet.end()};
}

auto DictionaryChi::Upper_bound(const std::string& key, const std::span<const Key>& characterSet)
        -> std::span<const Key>
{
    return Upper_bound(std::string_view(key), characterSet);
}

auto DictionaryChi::Upper_bound(const std::string_view& key, const std::span<const Key>& characterSet)
        -> std::span<const Key>
{
    return {characterSet.begin(),
            ranges::upper_bound(characterSet, key, ranges::less{}, [&key](const Key& k) {
                return k.key.substr(0, key.length());
            })};
}

auto DictionaryChi::characterSetTypeFromKeySpan(const std::span<const Key>& keys) const -> CharacterSetType
{
    auto sameSpan = [](const std::span<const Key>& a, const std::span<const Key>& b) -> bool {
        return (a.begin() == b.begin()) && (a.end() == b.end());
    };
    if (sameSpan(keys, Simplified())) {
        return CharacterSetType::Simplified;
    }
    if (sameSpan(keys, Traditional())) {
        return CharacterSetType::Traditional;
    }
    throw std::invalid_argument("Invalid choice other than traditional / simplified!");
}

auto DictionaryChi::keySpanFromCharacterSetType(CharacterSetType characterSet) const -> std::span<const Key>
{
    switch (characterSet) {
    case CharacterSetType::Simplified:
        return Simplified();
    case CharacterSetType::Traditional:
        return Traditional();
    default:
        std::unreachable();
    }
}

auto DictionaryChi::EntryFromPosition(size_t pos, const std::span<const Key>& keys) const -> Entry
{
    const auto characterSet = characterSetTypeFromKeySpan(keys);
    const auto& pos_to_characterSet = (characterSet == CharacterSetType::Simplified)
                                              ? position_to_simplified
                                              : position_to_traditional;
    return {.key = keys[pos_to_characterSet[pos]].key,
            .pronounciation = pronounciation.at(pos),
            .meanings = meanings.at(pos),
            .id = static_cast<VocableId>(pos)};
}

auto DictionaryChi::entryFromPosition(size_t pos, CharacterSetType characterSet) const -> Entry
{
    switch (characterSet) {
    case CharacterSetType::Simplified:
        return EntryFromPosition(pos, simplified);
    case CharacterSetType::Traditional:
        return EntryFromPosition(pos, traditional);
    default:
        std::unreachable();
    }
}

auto DictionaryChi::entryVectorFromKey(const std::string& key) const -> std::vector<Entry>
{
    std::vector<Entry> entries;
    // ToDo: it should be possible to support both, simplified and traditional at the same time.
    //   no need to only extract the entryVector from simplified/traditional only. No it supports simplified only
    const auto span_lower = DictionaryChi::Lower_bound(key, Simplified());
    const auto span_now = DictionaryChi::Upper_bound(key, span_lower);
    for (const auto& k : span_now) {
        if (key != k.key) {
            continue;
        }
        entries.push_back({.key = key,
                           .pronounciation = pronounciation.at(k.pos),
                           .meanings = meanings.at(k.pos),
                           .id = static_cast<VocableId>(k.pos)});
    }

    return entries;
}

auto DictionaryChi::contains(const std::string& key) const -> bool
{
    const auto span_lower = DictionaryChi::Lower_bound(key, Simplified());
    const auto span_now = DictionaryChi::Upper_bound(key, span_lower);
    if (span_now.empty()) {
        return false;
    }

    return ranges::any_of(span_now, [&](const auto& k) { return k.key == key; });
}

auto DictionaryChi::posFromKey(const std::string& key) const -> unsigned
{
    const auto span_lower = DictionaryChi::Lower_bound(key, Simplified());
    const auto span_now = DictionaryChi::Upper_bound(key, span_lower);
    for (const auto& k : span_now) {
        if (key == k.key) {
            return k.pos;
        }
    }
    return size();
}

auto DictionaryChi::size() const -> unsigned
{
    return static_cast<unsigned>(simplified.size());
}

auto DictionaryChi::Entry::operator<=>(const Entry& other) const -> std::weak_ordering
{
    if (const auto cmp = key <=> other.key; cmp != nullptr) {
        return cmp;
    }
    if (const auto cmp = pronounciation <=> other.pronounciation; cmp != nullptr) {
        return cmp;
    }
    return meanings <=> other.meanings;
}
} // namespace dictionary
