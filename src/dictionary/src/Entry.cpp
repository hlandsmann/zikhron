#include "Entry.h"
#include <compare>

namespace dictionary {

auto Entry::operator<=>(const Entry& other) const -> std::weak_ordering
{
    if (const auto cmp = key <=> other.key; cmp != nullptr) {
        return cmp;
    }
    if (const auto cmp = pronounciation <=> other.pronounciation; cmp != nullptr) {
        return cmp;
    }
    return meanings <=> other.meanings;
}

} // namespace dictionary
