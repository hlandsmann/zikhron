#include <Token.h>
#include <database/Word.h>
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
Token::Token(utl::StringU8 _value)
    : value{std::move(_value)}
{}

Token::Token(utl::StringU8 _value, std::shared_ptr<database::Word> _word)
    : value{std::move(_value)}
    , word{std::move(_word)}
{}

Token::Token(utl::StringU8 _value, EntryVector _dictionaryEntries)
    : value{std::move(_value)}
    , dictionaryEntries{std::move(_dictionaryEntries)}
{}

auto Token::getWord() const -> std::shared_ptr<database::Word>
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
    if (word) {
        return word->getId();
    }
    return {};
}

auto Token::getDictionaryEntries() const -> const EntryVector&
{
    return dictionaryEntries;
}

void Token::resetWord()
{
    word.reset();
}

auto Token::string() const -> std::string
{
    return value.string();
}

Token::operator std::string() const
{
    return value;
}

auto Token::fromStringU8(const utl::StringU8& str) -> Token
{
    return {str};
}

auto tokenVectorFromString(const std::string& str, ColorId colorId) -> std::vector<Token>
{
    std::vector<Token> result;

    auto ss = std::stringstream{str};
    std::istream_iterator<std::string> first(ss);
    std::vector<std::string> vstrings(first, std::istream_iterator<std::string>{});
    ranges::transform(vstrings, std::back_inserter(result),
                      [colorId](const std::string& tokenStr) -> Token {
                          auto token = Token{tokenStr};

                          token.setColorId(colorId);
                          return token;
                      });
    return result;
}

} // namespace annotation
