#pragma once
#include <fmt/format.h>

#include <compare>
#include <cstddef>
#include <format>
#include <span>
#include <string>
#include <utility>
#include <vector>
namespace icu {
class UnicodeString;
}

namespace utl {

template<class T>
auto stringPlusT(std::string&& a, const T& b) -> std::string
{
    return std::move(a) += b;
}

class CharU8
{
public:
    CharU8(std::string _str, bool _isMarkup, size_t _virtualLength)
        : str{std::move(_str)}, markup{_isMarkup}, virtualLength{_virtualLength} {};
    CharU8(std::string _str)
        : str{std::move(_str)} {};
    CharU8(const CharU8&) = default;
    CharU8(CharU8&&) = default;
    ~CharU8() = default;
    auto operator=(const CharU8& other) -> CharU8& = default;
    auto operator=(CharU8&& other) noexcept -> CharU8& = default;
    operator std::string() const { return str; }
    [[nodiscard]] auto string() const -> std::string { return str; }
    [[nodiscard]] auto vLength() const -> size_t { return virtualLength; }
    [[nodiscard]] auto isMarkup() const -> bool { return markup; }
    auto operator<=>(const CharU8& other) const -> std::weak_ordering { return str <=> other.str; };
    auto operator==(const CharU8& other) const -> bool = default;

private:
    std::string str;
    bool markup = false;
    size_t virtualLength = 1;
};

class StringU8
{
    std::vector<CharU8> chars;

public:
    StringU8() = default;
    ~StringU8() = default;
    StringU8(const std::string&);
    StringU8(const std::span<const CharU8>&);
    StringU8(const icu::UnicodeString&);
    StringU8(const StringU8&) = default;
    StringU8(StringU8&&) = default;
    auto operator=(const StringU8& other) -> StringU8& = default;
    auto operator=(StringU8&& other) noexcept -> StringU8& = default;
    auto operator<=>(const StringU8&) const -> std::weak_ordering;
    operator std::string() const;
    [[nodiscard]] auto string() const -> std::string;

    [[nodiscard]] auto length() const -> size_t;
    [[nodiscard]] auto vlength() const -> size_t;
    [[nodiscard]] auto empty() const -> bool;
    [[nodiscard]] auto at(size_t pos) const -> StringU8;
    [[nodiscard]] auto substr(long pos, long n) const -> std::string;
    [[nodiscard]] auto back() const -> CharU8;
    [[nodiscard]] auto front() const -> CharU8;
    [[nodiscard]] auto cbegin() const { return chars.cbegin(); }
    [[nodiscard]] auto cend() const { return chars.cend(); }
    void push_back(const CharU8&);
    void append(const std::string&);
    void append(const icu::UnicodeString&);

private:
    static auto icustringToString(const icu::UnicodeString& str) -> std::string;
};

} // namespace utl

template<>
struct std::formatter<utl::CharU8>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }
    template<typename FormatContext>
    auto format(const utl::CharU8& itemU8, FormatContext& ctx)
    {
        return std::format_to(ctx.out(), "{}", std::string(itemU8));
    }
};
