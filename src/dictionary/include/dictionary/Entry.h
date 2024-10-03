#pragma once
#include <compare>
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

struct Entry
{
    std::string key;
    std::string pronounciation;
    std::vector<std::string> meanings;

    auto operator<=>(const Entry& other) const -> std::weak_ordering;

    auto operator==(const Entry&) const -> bool = default;
};
} // namespace dictionary
