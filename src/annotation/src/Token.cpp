#include <Token.h>
#include <misc/Identifier.h>
#include <utils/StringU8.h>

#include <string>
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
    if (colorId == 0) {
        return colorId;
    }
    return static_cast<ColorId>((colorId - 1) % maxId + 1);
}

void Token::setColorId(unsigned _colorId)
{
    colorId = static_cast<ColorId>(_colorId);
}

auto Token::string() const -> std::string
{
    return value.string();
}

Token::operator std::string() const
{
    return value;
}

} // namespace annotation
