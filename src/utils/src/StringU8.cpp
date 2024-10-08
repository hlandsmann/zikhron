#include "StringU8.h"

#include <unicode/unistr.h>
#include <unicode/utf.h>
#include <unicode/utf8.h>

#include <algorithm>
#include <compare>
#include <cstddef>
#include <iterator>
#include <numeric>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ranges = std::ranges;

namespace utl {

StringU8::StringU8(const std::string& str)
{
    append(str);
}

StringU8::StringU8(std::string_view sv)
{
    append(std::string{sv});
}

StringU8::StringU8(const std::span<const CharU8>& items)
{
    ranges::copy(items, std::back_inserter(chars));
}

StringU8::operator std::string() const
{
    return string();
}

auto StringU8::string() const -> std::string
{
    return substr(0, static_cast<long>(chars.size()));
}

auto StringU8::operator<=>(const StringU8& other) const -> std::weak_ordering
{
    return chars <=> other.chars;
}

auto StringU8::operator==(const StringU8& other) const -> bool
{
    return chars == other.chars;
}

auto StringU8::length() const -> size_t
{
    return chars.size();
}

auto StringU8::vlength() const -> size_t
{
    return std::accumulate(
            chars.cbegin(), chars.cend(), size_t(0), [](const size_t a, const CharU8& b) -> size_t {
                return a + b.vLength();
            });
}

auto StringU8::empty() const -> bool
{
    return chars.empty();
}

auto StringU8::at(size_t pos) const -> CharU8
{
    return chars.at(pos);
}

auto StringU8::substr(long pos, long n) const -> std::string
{
    auto first = std::min(std::next(chars.begin(), pos), chars.end());
    auto last = std::min(std::next(chars.begin(), pos + n), chars.end());
    return std::accumulate(first, last, std::string{}, stringPlusT<CharU8>);
}

auto StringU8::back() const -> const CharU8&
{
    return chars.back();
}

auto StringU8::front() const -> const CharU8&
{
    return chars.front();
}

void StringU8::push_back(const CharU8& itemU8)
{
    chars.push_back(itemU8);
}

void StringU8::append(const std::string& str)
{
    size_t i = 0;
    size_t old_i = 0;
    while (i < str.length()) {
        U8_FWD_1(str, i, str.length());
        chars.emplace_back(str.substr(old_i, i - old_i));
        old_i = i;
    }
}

auto concanateStringsU8(const std::vector<StringU8>& strings) -> StringU8
{
    return std::accumulate(strings.begin(), strings.end(), std::string{}, stringPlusT<StringU8>);
}

auto stringU8VectorFromStrings(const std::vector<std::string>& strings) -> std::vector<StringU8>
{
    auto stringU8Vector = std::vector<StringU8>{};
    ranges::copy(strings, std::back_inserter(stringU8Vector));
    return stringU8Vector;
}

} // namespace utl
