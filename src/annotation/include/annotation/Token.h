#pragma once
#include <database/Word.h>
#include <misc/Identifier.h>
#include <utils/StringU8.h>
#include <utils/format.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace annotation {
enum class NoBreak {
    front,
    back,
    none
};

class Token
{
public:
    Token() = default;
    Token(utl::StringU8 value);
    Token(utl::StringU8 value, std::shared_ptr<database::Word> word);
    [[nodiscard]] auto getWord() const -> std::shared_ptr<database::Word>;
    [[nodiscard]] auto getValue() const -> utl::StringU8;
    [[nodiscard]] auto getNoBreak() const -> NoBreak;
    [[nodiscard]] auto getColorId() const -> ColorId;
    void setColorId(ColorId colorId);
    [[nodiscard]] auto getVocableId() const -> std::optional<VocableId>;
    void resetWord();

    [[nodiscard]] auto string() const -> std::string;
    operator std::string() const;

    static auto fromStringU8(const utl::StringU8& str) -> Token;

private:
    utl::StringU8 value;
    std::shared_ptr<database::Word> word;
    ColorId colorId{ColorId::defaultFontColor};
    NoBreak noBreak{NoBreak::none};
};

auto tokenVectorFromString(const std::string& str, ColorId colorId = ColorId::defaultFontColor)
        -> std::vector<Token>;
} // namespace annotation

template<>
struct fmt::formatter<annotation::Token> : fmt::formatter<std::string>
{
    auto format(annotation::Token const& token, format_context& ctx) const
    {
        return formatter<std::string>::format(token.string(), ctx);
    }

};
