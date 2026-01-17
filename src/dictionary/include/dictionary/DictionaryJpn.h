#pragma once
#include "Dictionary.h"
#include "Key_jpn.h"

#include <misc/Config.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <utils/format.h>

#include <compare>
#include <cstddef>
#include <magic_enum/magic_enum.hpp>
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

struct EntryJpn
{
    std::string key;
    std::vector<std::string> pronounciation;
    std::vector<std::string> meanings;

    auto operator<=>(const EntryJpn& other) const -> std::weak_ordering
    {
        if (const auto cmp = key <=> other.key; cmp != nullptr) {
            return cmp;
        }
        if (const auto cmp = pronounciation <=> other.pronounciation; cmp != nullptr) {
            return cmp;
        }
        return meanings <=> other.meanings;
    }

    auto operator==(const EntryJpn&) const -> bool = default;
};

class DictionaryJpn : public Dictionary
{
public:
    DictionaryJpn(std::shared_ptr<zikhron::Config> config);
    [[nodiscard]] auto entriesFromKey(const std::string& key) const -> std::vector<EntryJpn>;
    [[nodiscard]] auto contains(const std::string& key) const -> bool override;

    struct Definition
    {
        std::set<std::string> readings;
        std::set<std::string> glossary;
        std::set<std::string> infos;
        std::set<PartOfSpeech> pos;
        auto operator==(const Definition&) const -> bool = default;
    };

    struct InternalEntry
    {
        std::vector<std::string> kanjis;
        std::vector<Definition> definitions;

        operator bool() const { return (!definitions.empty()); }
    };

    [[nodiscard]] auto getEntryByKey(const Key_jpn& key) const -> InternalEntry;
    [[nodiscard]] auto getEntryByKanji(const std::string& key) const -> InternalEntry;
    [[nodiscard]] auto getEntryByReading(const std::string& key) const -> InternalEntry;
    [[nodiscard]] auto getEntryByReading(const std::string& reading, const std::string& hint) const -> std::vector<InternalEntry>;
    [[nodiscard]] auto getEntryByKanji(const std::string& kanji,
                                       const std::string& hint,
                                       const std::string& reading) const -> InternalEntry;
    void setDebugSink(spdlog::sink_ptr sink) override;
    std::map<std::string, std::vector<std::size_t>> kanjiToIndex;

private:
    std::vector<InternalEntry> entries;

    struct ReadingPosition
    {
        std::size_t indexEntry;
        std::size_t indexDefinition;
    };

    std::map<std::string, std::vector<ReadingPosition>> readingToIndex;

    std::unique_ptr<spdlog::logger> log;
};
} // namespace dictionary

template<>
struct fmt::formatter<dictionary::PartOfSpeech>

{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(dictionary::PartOfSpeech orientation, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", magic_enum::enum_name(orientation));
    }
};
