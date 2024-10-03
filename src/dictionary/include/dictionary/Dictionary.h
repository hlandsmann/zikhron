#pragma once
#include "Entry.h"

#include <string>
#include <vector>

namespace dictionary {

class Dictionary
{
public:
    Dictionary() = default;
    virtual ~Dictionary() = default;
    Dictionary(const Dictionary&) = default;
    Dictionary(Dictionary&&) = default;
    auto operator=(const Dictionary&) -> Dictionary& = default;
    auto operator=(Dictionary&&) -> Dictionary& = default;

    [[nodiscard]] virtual auto entriesFromKey(const std::string& key) const -> std::vector<Entry> = 0;
    [[nodiscard]] virtual auto contains(const std::string& key) const -> bool = 0;
};
} // namespace dictionary
