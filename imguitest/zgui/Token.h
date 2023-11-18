#pragma once
#include <utils/StringU8.h>

#include <format>
#include <string>
#include <utility>

class Token
{
public:
    Token(utl::StringU8 token);
    Token(std::string token);
    Token(const Token&) = default;
    Token(Token&&) = default;
    ~Token() = default;
    auto operator=(const Token& word) noexcept -> Token& = default;
    auto operator=(Token&& word) noexcept -> Token& = default;

    [[nodiscard]] auto string() const -> std::string&;
    operator std::string() const;

    void setColorId(unsigned colorId);
    [[nodiscard]] auto getColorId() const -> unsigned;

private:
    std::string token;
    unsigned colorId{};
};

template<>
struct std::formatter<Token>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }
    template<typename FormatContext>
    auto format(const Token& token, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}", token.string());
    }
};

inline auto operator+(std::string&& str, const Token& token) -> std::string
{
    return std::move(str) + std::string(token);
}
