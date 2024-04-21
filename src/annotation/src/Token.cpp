#include <Token.h>
#include <Word.h>
#include <misc/Identifier.h>
#include <utils/StringU8.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace annotation {
Token::Token(utl::StringU8 _value, ZH_dicItemVec _dictionaryEntries)
    : value{std::move(_value)}
    , dictionaryEntries{std::move(_dictionaryEntries)}
{}

auto Token::getWord() const -> std::shared_ptr<Word>
{
    return word;
}

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

auto tokenVectorFromString(const std::string& str, ColorId colorId) -> std::vector<Token>
{
    std::vector<Token> result;

    auto ss = std::stringstream{str};
    std::istream_iterator<std::string> first(ss);
    std::vector<std::string> vstrings(first, std::istream_iterator<std::string>{});
    ranges::transform(vstrings, std::back_inserter(result),
                      [colorId](const std::string& tokenStr) -> Token {
                          auto token = Token{tokenStr, {}};

                          token.setColorId(colorId);
                          return token;
                      });
    return result;
}

} // namespace annotation
