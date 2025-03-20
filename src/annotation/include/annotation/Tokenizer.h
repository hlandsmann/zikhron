#pragma once
#include <spdlog/common.h>
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

    virtual void setDebugSink(spdlog::sink_ptr /* sink */) {};

};
} // namespace annotation
