#include "StringU8.h"
#include <unicode/unistr.h>
#include <algorithm>
#include <concepts>
#include <iterator>
#include <numeric>
#include <unicode/utf.h>

namespace ranges = std::ranges;
namespace utl {

// StringU8::StringU8(const std::string_view& _strv) : StringU8(std::string(_strv)) {}
StringU8::StringU8(const icu::UnicodeString& _str) : StringU8(icustringToString(_str)) {}
StringU8::StringU8(const std::string& str) { append(str); }
StringU8::StringU8(const std::span<const ItemU8>& items) {
    ranges::copy(items, std::back_inserter(chars));
}

StringU8::operator std::string() const { return substr(0, chars.size()); }
auto StringU8::operator=(const utl::StringU8& other) -> StringU8& {
    chars = other.chars;
    return *this;
}

auto StringU8::operator<=>(const StringU8& other) const -> std::weak_ordering {
    return chars <=> other.chars;
}

auto StringU8::length() const -> size_t { return chars.size(); }

auto StringU8::vlength() const -> size_t {
    return std::accumulate(
        chars.cbegin(), chars.cend(), size_t(0), [](const size_t a, const ItemU8& b) -> size_t {
            return a + b.vLength();
        });
}
auto StringU8::empty() const -> bool { return chars.empty(); }
auto StringU8::at(size_t pos) const -> StringU8 {
    StringU8 strU8;
    strU8.push_back(chars.at(pos));
    return strU8;
}
auto StringU8::substr(size_t pos, size_t n) const -> std::string {
    auto first = std::min(std::next(chars.begin(), pos), chars.end());
    auto last = std::min(std::next(chars.begin(), pos + n), chars.end());
    return std::accumulate(first, last, std::string{}, stringPlusT<ItemU8>);
}
auto StringU8::back() const -> ItemU8 { return chars.back(); }
auto StringU8::front() const -> ItemU8 { return chars.front(); }

auto StringU8::icustringToString(const icu::UnicodeString& _str) -> std::string {
    std::string tempString;
    _str.toUTF8String(tempString);
    return tempString;
}

void StringU8::push_back(const ItemU8& itemU8) { chars.push_back(itemU8); }
void StringU8::append(const std::string& str) {
    size_t i = 0, old_i = 0;
    while (i < str.length()) {
        U8_FWD_1(str, i, str.length());
        chars.push_back(str.substr(old_i, i - old_i));
        old_i = i;
    }
}
void StringU8::append(const icu::UnicodeString& uniStr) {
    const std::string str = icustringToString(uniStr);
    append(str);
}
}  // namespace utl
