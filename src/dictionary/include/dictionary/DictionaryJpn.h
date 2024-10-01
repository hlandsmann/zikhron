#pragma once
#include <misc/Config.h>

#include <cstddef>
#include <filesystem>
#include <map>
#include <memory>
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
    std::vector<std::string> kanji;
    std::vector<Definition> definition;
};

class DictionaryJpn
{
public:
    DictionaryJpn(std::shared_ptr<zikhron::Config> config);
    [[nodiscard]] auto getEntryByKanji(const std::string& key) const -> Entry;
    [[nodiscard]] auto getEntryByReading(const std::string& key) const -> Entry;

private:
    std::vector<Entry> entries;
    std::map<std::string, std::vector<std::size_t>> kanjiToIndex;

    struct ReadingPosition
    {
        std::size_t indexEntry;
        std::size_t indexDefinition;
    };

    std::map<std::string, std::vector<ReadingPosition>> readingToIndex;
};
} // namespace dictionary
