#pragma once

#include <string>

namespace dictionary {
auto katakanaToHirigana(const std::string& katakana) -> std::string;
auto hiriganaToKatakana(const std::string& hirigana) -> std::string;
auto katakanaToRomanji(const std::string& katakana) -> std::string;
auto hiriganaToRomanji(const std::string& hirigana) -> std::string;
} // namespace dictionary
