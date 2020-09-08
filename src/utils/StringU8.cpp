#include <unicode/unistr.h>
#include <stdexcept>

#include "StringU8.h"
namespace utl {
StringU8::StringU8(const std::string_view& _strv) : StringU8(std::string(_strv)){}
StringU8::StringU8(const icu::UnicodeString& _str) : StringU8(icustringToString(_str)) {}
StringU8::StringU8(const std::string& _str) : str(_str), charPos(genCharPos()) {}

size_t StringU8::length() const { return charPos.size(); }
bool StringU8::empty() const { return charPos.empty(); }
std::string_view StringU8::at(size_t pos) const {
    size_t firstPos = getAbsoluteStrPosition(pos);
    size_t lastPos = getAbsoluteStrPosition(pos + 1);
    return std::string_view(str).substr(firstPos, lastPos - firstPos);
}
std::string_view StringU8::substr(size_t pos, size_t n) const {
    size_t firstPos = getAbsoluteStrPosition(pos);
    size_t lastPos = getAbsoluteStrPosition(pos + n);
    return std::string_view(str).substr(firstPos, lastPos - firstPos);
}
std::string_view StringU8::back() const {
    if (empty())
        return std::string_view();
    return substr(length() - 1, 1);
}
std::string_view StringU8::front() const {
    if (empty())
        return std::string_view();
    return substr(0, 1);
}
void StringU8::iterate_characters(std::function<void(const std::string_view&)> op,
                                  int first,
                                  int last) const {
    first = std::max(first, 0);
    last = std::min<int>(length(), last);
    if (last < 0)
        last = std::max<int>(0, length() + last);
    for (int i = first; i < last; i++)
        op(substr(i, 1));
}
StringU8::operator std::string() const { return str; }

size_t StringU8::getAbsoluteStrPosition(size_t pos) const {
    if (pos > charPos.size())
        throw std::out_of_range(std::to_string(pos) + " is bigger than max " +
                                std::to_string(charPos.size() - 1));
    if (pos == charPos.size())
        return str.length();
    return charPos[pos];
}
std::vector<size_t> StringU8::genCharPos() {
    std::vector<size_t> _charPos;
    size_t i = 0;
    while (i < str.length()) {
        _charPos.push_back(i);
        U8_FWD_1(str, i, str.length());
    }
    _charPos.shrink_to_fit();
    return _charPos;
}
std::string StringU8::icustringToString(const icu::UnicodeString& _str) {
    std::string tempString;
    _str.toUTF8String(tempString);
    return tempString;
}
}  // namespace utl
