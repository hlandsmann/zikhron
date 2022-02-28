#pragma once

#include <fmt/format.h>
#include <stddef.h>
#include <compare>
#include <iosfwd>
#include <span>
#include <string>
#include <vector>
namespace icu {
class UnicodeString;
}

namespace utl {

template <class T> std::string stringPlusT(std::string&& a, const T& b) { return a += b; }

/* Item representing either a character or a markup formatter */
class CharU8 {
public:
    CharU8(const std::string& _str, bool _isMarkup, size_t _virtualLength)
        : str(_str), markup(_isMarkup), virtualLength(_virtualLength){};
    CharU8(const std::string& _str) : str(_str){};
    CharU8(const CharU8&) = default;
    operator std::string() const { return str; }
    auto vLength() const -> size_t { return virtualLength; }
    auto isMarkup() const -> bool { return markup; }
    auto operator<=>(const CharU8& other) const -> std::weak_ordering { return str <=> other.str; };
    auto operator==(const CharU8& other) const -> bool = default;

private:
    std::string str;
    bool markup = false;
    size_t virtualLength = 1;
};

class StringU8 {
    std::vector<CharU8> chars;

public:
    StringU8() = default;
    StringU8(const std::string&);
    StringU8(const std::span<const CharU8>&);
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
    auto back() const -> CharU8;
    auto front() const -> CharU8;
    auto cbegin() const { return chars.cbegin(); }
    auto cend() const { return chars.cend(); }
    // void push_back(const std::string&);
    void push_back(const CharU8&);
    void append(const std::string&);
    void append(const icu::UnicodeString&);

private:
    auto icustringToString(const icu::UnicodeString& str) -> std::string;
};

}  // namespace utl
inline std::ostream& operator<<(std::ostream& os, const utl::CharU8& itemU8) {
    os << std::string(itemU8);
    return os;
}
template <> struct fmt::formatter<utl::CharU8> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }
    template <typename FormatContext> auto format(const utl::CharU8& itemU8, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}", std::string(itemU8));
    }
};

inline std::ostream& operator<<(std::ostream& os, const utl::StringU8& strU8) {
    os << std::string(strU8);
    return os;
}
