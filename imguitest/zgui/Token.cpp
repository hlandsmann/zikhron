#include <Token.h>
#include <utils/StringU8.h>

#include <string>
#include <utility>

Token::Token(utl::StringU8 _token)
    : Token{_token.string()}
{
}

Token::Token(std::string _token)
    : token{std::move(_token)}
{
}
