#pragma once
#include <functional>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

namespace icu {
class UnicodeString;
}
namespace utl {
class StringU8 {
public:
    StringU8(const std::string&);
    StringU8(const std::string_view&);
    StringU8(const icu::UnicodeString&);
    StringU8(const StringU8&) = default;
    StringU8(StringU8&&) = default;
    size_t length() const;
    bool empty() const;
    std::string_view at(size_t pos) const;
    std::string_view substr(size_t pos, size_t n) const;
    std::string_view back() const;
    std::string_view front() const;
    void iterate_characters(std::function<void(const std::string_view&)> op,
                            int first = 0,
                            int last = std::numeric_limits<int>::max()) const;
    operator std::string() const;

private:
    size_t getAbsoluteStrPosition(size_t pos) const;
    std::vector<size_t> genCharPos();
    std::string icustringToString(const icu::UnicodeString& str);
    const std::string str;
    std::vector<size_t> charPos;
};

}  // namespace utl

inline std::ostream& operator<<(std::ostream& os, const utl::StringU8& strU8) {
    os << std::string(strU8);
    return os;
}
