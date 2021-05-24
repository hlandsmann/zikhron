#pragma once

#include <span>
#include <string>
#include <vector>

class ZH_Dictionary {
public:
    struct Key {
        std::string key;
        unsigned pos;
    };
    ZH_Dictionary(const std::string& filename);
    static auto Lower_bound(const std::string& key, const std::span<const Key>& characterSet)
        -> std::span<const Key>;
    static auto Lower_bound(const std::string_view& key, const std::span<const Key>& characterSet)
        -> std::span<const Key>;
    static auto Upper_bound(const std::string& key, const std::span<const Key>& characterSet)
        -> std::span<const Key>;
    static auto Upper_bound(const std::string_view& key, const std::span<const Key>& characterSet)
        -> std::span<const Key>;

    enum class CharacterSet { Simplified, Traditional };
    auto Simplified() const -> std::span<const Key> { return simplified; }
    auto Traditional() const -> std::span<const Key> { return traditional; }

    struct Item {
        std::string key;
        std::string pronounciation;
        std::vector<std::string> meanings;
        unsigned id;
        auto operator<=>(const Item&) const -> std::weak_ordering;
        bool operator==(const Item&) const = default;
    };
    auto CharacterSetFromKeySpan(const std::span<const Key>& keys) const -> CharacterSet;
    auto ItemFromPosition(size_t pos, const std::span<const Key>& keys) const -> Item;
    auto ItemFromPosition(size_t pos, CharacterSet characterSet) const -> Item;

private:
    std::vector<Key> traditional;
    std::vector<Key> simplified;
    std::vector<std::string> pronounciation;
    std::vector<std::vector<std::string>> meanings;

    std::vector<unsigned> position_to_simplified;
    std::vector<unsigned> position_to_traditional;
};
