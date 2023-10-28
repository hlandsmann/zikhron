#pragma once
#include <misc/Identifier.h>

#include <compare>
#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

enum class CharacterSetType {
    Simplified,
    Traditional
};

class ZH_Dictionary
{
public:
    struct Key
    {
        std::string key;
        unsigned pos;
    };
    ZH_Dictionary(const std::filesystem::path&);
    static auto Lower_bound(const std::string& key, const std::span<const Key>& characterSet)
            -> std::span<const Key>;
    static auto Lower_bound(const std::string_view& key, const std::span<const Key>& characterSet)
            -> std::span<const Key>;
    static auto Upper_bound(const std::string& key, const std::span<const Key>& characterSet)
            -> std::span<const Key>;
    static auto Upper_bound(const std::string_view& key, const std::span<const Key>& characterSet)
            -> std::span<const Key>;

    [[nodiscard]] auto Simplified() const -> std::span<const Key> { return simplified; }
    [[nodiscard]] auto Traditional() const -> std::span<const Key> { return traditional; }

    struct Entry
    {
        std::string key;
        std::string pronounciation;
        std::vector<std::string> meanings;
        VocableId id;
        auto operator<=>(const Entry&) const -> std::weak_ordering;
        auto operator==(const Entry&) const -> bool = default;
    };
    [[nodiscard]] auto CharacterSetTypeFromKeySpan(const std::span<const Key>& keys) const -> CharacterSetType;
    [[nodiscard]] auto EntryFromPosition(size_t pos, CharacterSetType characterSet) const -> Entry;

private:
    [[nodiscard]] auto EntryFromPosition(size_t pos, const std::span<const Key>& keys) const -> Entry;
    std::vector<Key> traditional;
    std::vector<Key> simplified;
    std::vector<std::string> pronounciation;
    std::vector<std::vector<std::string>> meanings;

    std::vector<unsigned> position_to_simplified;
    std::vector<unsigned> position_to_traditional;
};
