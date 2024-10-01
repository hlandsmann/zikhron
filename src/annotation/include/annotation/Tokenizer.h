#pragma once
#include "Token.h"

#include <string>
#include <vector>

namespace annotation {
class Tokenizer
{
public:
    Tokenizer() = default;
    virtual ~Tokenizer() = default;
    Tokenizer(const Tokenizer&) = default;
    Tokenizer(Tokenizer&&) = default;
    auto operator=(const Tokenizer&) -> Tokenizer& = default;
    auto operator=(Tokenizer&&) -> Tokenizer& = default;

    [[nodiscard]] virtual auto split(const std::string& /* text */) const -> std::vector<Token> = 0; //{ return {}; };
};
} // namespace annotation
