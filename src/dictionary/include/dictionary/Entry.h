#pragma once
#include <utils/format.h>

#include <compare>
#include <magic_enum/magic_enum.hpp>
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
