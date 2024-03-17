#include <Token.h>
#include <misc/Identifier.h>
#include <utils/StringU8.h>

#include <optional>
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

auto Token::getColorId() const -> ColorId
{
    return colorId;
}

void Token::setColorId(ColorId _colorId)
{
    colorId = _colorId;
}

auto Token::getVocableId() const -> std::optional<VocableId>
{
    return vocableId;
}

void Token::setVocableId(VocableId _vocableId)
{
    vocableId.emplace(_vocableId);
}

auto Token::getDictionaryEntries() const -> const ZH_dicItemVec&
{
    return dictionaryEntries;
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
