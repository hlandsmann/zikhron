#pragma once

#include <compare>
#include <functional>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace icu {
class UnicodeString;
}

namespace utl {

template <class T> std::string stringPlusT(std::string&& a, const T& b) { return a += b; }

/* Item representing either a character or a markup formatter */
class ItemU8 {
public:
    ItemU8(const std::string& _str, bool _isMarkup, size_t _virtualLength)
        : str(_str), markup(_isMarkup), virtualLength(_virtualLength){};
    ItemU8(const std::string& _str) : str(_str){};
    ItemU8(const ItemU8&) = default;
    operator std::string() const { return str; }
    auto vLength() const -> size_t { return virtualLength; }
    auto isMarkup() const -> bool { return markup; }
    auto operator<=>(const ItemU8& other) const -> std::weak_ordering { return str <=> other.str; };

private:
    std::string str;
    bool markup = false;
    size_t virtualLength = 1;
};

class StringU8 {
    std::vector<ItemU8> chars;

public:
    StringU8() = default;
    StringU8(const std::string&);
    StringU8(const std::span<const ItemU8>&);
    // StringU8(const std::string_view&);
    StringU8(const icu::UnicodeString&);
    StringU8(const StringU8&) = default;
    StringU8(StringU8&&) = default;
    auto operator=(const utl::StringU8& other) -> StringU8&;
    auto operator<=>(const StringU8&) const -> std::weak_ordering;
    operator std::string() const;

    auto length() const -> size_t;
    auto vlength() const -> size_t;
    auto empty() const -> bool;
    auto at(size_t pos) const -> StringU8;
    auto substr(size_t pos, size_t n) const -> std::string;
    auto back() const -> ItemU8;
    auto front() const -> ItemU8;
    auto cbegin() const { return chars.cbegin(); }
    auto cend() const { return chars.cend(); }
    // void push_back(const std::string&);
    void push_back(const ItemU8&);
    void append(const std::string&);
    void append(const icu::UnicodeString&);

private:
    auto icustringToString(const icu::UnicodeString& str) -> std::string;
};

}  // namespace utl
inline std::ostream& operator<<(std::ostream& os, const utl::ItemU8& itemU8) {
    os << std::string(itemU8);
    return os;
}
inline std::ostream& operator<<(std::ostream& os, const utl::StringU8& strU8) {
    os << std::string(strU8);
    return os;
}
