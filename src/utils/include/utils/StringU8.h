#pragma once
#include "format.h"

#include <fmt/format.h> // IWYU pragma: export core.h

#include <compare>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
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

    [[nodiscard]] auto string() const -> const std::string& { return str; }

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
    StringU8(std::string_view);

    StringU8(const std::span<const CharU8>&);
    StringU8(const StringU8&) = default;
    StringU8(StringU8&&) = default;
    auto operator=(const StringU8& other) -> StringU8& = default;
    auto operator=(StringU8&& other) noexcept -> StringU8& = default;
    auto operator<=>(const StringU8&) const -> std::weak_ordering;
    auto operator==(const StringU8&) const -> bool;
    operator std::string() const;

    auto operator+=(const StringU8& rhs) -> StringU8&
    {
        chars.insert(chars.end(), rhs.chars.begin(), rhs.chars.end());
        return *this;
    }

    auto operator+=(const CharU8& rhs) -> StringU8&
    {
        chars.insert(chars.end(), rhs);
        return *this;
    }

    friend auto operator+(StringU8 lhs, const StringU8& rhs) -> StringU8
    {
        lhs += rhs;
        return lhs;
    }

    [[nodiscard]] auto string() const -> std::string;

    [[nodiscard]] auto length() const -> size_t;
    [[nodiscard]] auto vlength() const -> size_t;
    [[nodiscard]] auto empty() const -> bool;
    [[nodiscard]] auto at(size_t pos) const -> CharU8;
    [[nodiscard]] auto substr(long pos, long n) const -> std::string;
    [[nodiscard]] auto back() const -> const CharU8&;
    [[nodiscard]] auto front() const -> const CharU8&;

    [[nodiscard]] auto getChars() const -> const std::vector<utl::CharU8>& { return chars; }

    [[nodiscard]] auto cbegin() const { return chars.cbegin(); }

    [[nodiscard]] auto cend() const { return chars.cend(); }

    void push_back(const CharU8&);
    void append(const std::string&);

private:
};

auto concanateStringsU8(const std::vector<StringU8>& strings) -> StringU8;
auto stringU8VectorFromStrings(const std::vector<std::string>& strings) -> std::vector<StringU8>;
} // namespace utl

template<>
struct fmt::formatter<utl::CharU8> : fmt::formatter<std::string>
{};

template<>
struct fmt::formatter<utl::StringU8> : fmt::formatter<std::string>
{};
