#pragma once
#include <string>

namespace dictionary {

auto katakanaToRomanji(const std::string& katakana) -> std::string;
auto hiriganaToRomanji(const std::string& hirigana) -> std::string;

class Kana
{
public:
    [[nodiscard]] static auto isKana(const std::string& token) -> bool;
    [[nodiscard]] static auto isHirigana(const std::string& token) -> bool;
    [[nodiscard]] static auto isKatakana(const std::string& token) -> bool;
    [[nodiscard]] static auto katakanaToHirigana(const std::string& katakana) -> std::string;
    [[nodiscard]] static auto hiriganaToKatakana(const std::string& hirigana) -> std::string;

private:
};
} // namespace dictionary
