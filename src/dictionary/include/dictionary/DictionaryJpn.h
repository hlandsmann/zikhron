#pragma once
#include "Dictionary.h"
#include "Entry.h"

#include <misc/Config.h>

#include <cstddef>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace dictionary {

class DictionaryJpn : public Dictionary
{
public:
    DictionaryJpn(std::shared_ptr<zikhron::Config> config);
    [[nodiscard]] auto entriesFromKey(const std::string& key) const -> std::vector<Entry> override;
    [[nodiscard]] auto contains(const std::string& key) const -> bool override;

    struct Definition
    {
        std::set<std::string> reading;
        std::set<std::string> glossary;
        std::set<std::string> info;
        std::set<PartOfSpeech> pos;
        auto operator==(const Definition&) const -> bool = default;
    };

    struct InternalEntry
    {
        std::vector<std::string> kanji;
        std::vector<Definition> definition;
    };

    [[nodiscard]] auto getEntryByKanji(const std::string& key) const -> InternalEntry;
    [[nodiscard]] auto getEntryByReading(const std::string& key) const -> InternalEntry;

private:
    std::vector<InternalEntry> entries;
    std::map<std::string, std::vector<std::size_t>> kanjiToIndex;

    struct ReadingPosition
    {
        std::size_t indexEntry;
        std::size_t indexDefinition;
    };

    std::map<std::string, std::vector<ReadingPosition>> readingToIndex;
};
} // namespace dictionary
