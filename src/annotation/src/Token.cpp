#include <Token.h>
#include <utils/StringU8.h>

#include <utility>

namespace annotation {
Token::Token(utl::StringU8 _value, ZH_dicItemVec _dictionaryEntries)
    : value{std::move(_value)}
    , dictionaryEntries{std::move(_dictionaryEntries)}
{}

auto Token::getValue() const -> utl::StringU8
{
    return value;
}

auto Token::getNoBreak() const -> NoBreak
{
    return noBreak;
}

auto Token::getColorId(ColorId maxId) const -> ColorId
{
    if (color == 0) {
        return color;
    }
    return static_cast<ColorId>((color - 1) % maxId + 1);
}

} // namespace annotation
