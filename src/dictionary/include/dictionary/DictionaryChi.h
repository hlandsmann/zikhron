#pragma once
#include "Dictionary.h"
#include "Entry.h"

#include <misc/Config.h>
#include <misc/Identifier.h>

#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dictionary {
class DictionaryChi : public Dictionary
{
    static constexpr std::string_view s_fn_dictionary = "cedict_1_0_ts_utf-8_mdbg.u8";

public:
    DictionaryChi(std::shared_ptr<zikhron::Config> config);

    [[nodiscard]] auto entriesFromKey(const std::string& key) const -> std::vector<Entry> override;
    [[nodiscard]] auto contains(const std::string& key) const -> bool override;

    struct Key
    {
        std::string key;
        unsigned position;
    };
    enum class CharacterSetType {
        Simplified,
        Traditional
    };
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

    [[nodiscard]] auto characterSetTypeFromKeySpan(const std::span<const Key>& keys) const -> CharacterSetType;
    [[nodiscard]] auto size() const -> unsigned;

private:
    [[nodiscard]] auto EntryFromPosition(size_t pos, const std::span<const Key>& keys) const -> Entry;
    std::vector<Key> traditional;
    std::vector<Key> simplified;
    std::vector<std::string> pronounciation;
    std::vector<std::vector<std::string>> meanings;

    std::vector<unsigned> position_to_simplified;
    std::vector<unsigned> position_to_traditional;
};
} // namespace dictionary
