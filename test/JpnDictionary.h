#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace japaneseDic {
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
    std::vector<std::string> reading;
    std::vector<std::string> glossary;
    PartOfSpeech pos;
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

private:
    std::vector<Entry> entries;
};
} // namespace japaneseDic
