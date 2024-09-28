#pragma once
#include <cstddef>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace dictionary {
enum class PartOfSpeech {
    undefined,
    adjective,
    adverb,
    conjunction,
    interjection,
    noun,
    pronoun,
    prefix,
    particle,
    suffix,
    verb,
};

struct Definition
{
    std::set<std::string> reading;
    std::set<std::string> glossary;
    std::set<PartOfSpeech> pos;
    auto operator==(const Definition&) const -> bool = default;
};

struct Entry
{
    std::vector<std::string> key; // mostly kanji, but sometimes not
    std::vector<Definition> definition;
};

class JpnDictionary
{
public:
    JpnDictionary(const std::filesystem::path& xmlFile);
    [[nodiscard]] auto getEntryByKanji(const std::string& key) const -> Entry;

private:
    std::vector<Entry> entries;
    std::map<std::string, std::vector<std::size_t>> kanjiToIndex;
    std::map<std::string, std::vector<std::size_t>> readingToIndex;
};
} // namespace japaneseDic
