#include <Token.h>
#include <utility>
#include <utils/StringU8.h>
Token::Token(utl::StringU8 _token)
    : token{std::move(_token)}
{
}
