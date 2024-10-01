#pragma once
#include <dictionary/DictionaryChi.h>
#include <utils/StringU8.h>

#include <memory>
#include <string>

namespace annotation {

class Rules
{
public:
    Rules(std::shared_ptr<const dictionary::DictionaryChi> dictionary);

    [[nodiscard]] auto findRule(const std::string& word) const -> std::string;
    [[nodiscard]] auto approachRule(const std::string& word) const -> bool;

private:
    [[nodiscard]] auto AABB_rule(const utl::StringU8& u8Str) const -> std::string;
    [[nodiscard]] auto specialEnding_rule(const utl::StringU8& u8Str) const -> std::string;
    [[nodiscard]] auto longerWord_rule(const utl::StringU8& u8Str) const -> std::string;
    std::shared_ptr<const dictionary::DictionaryChi> dictionary;
};

} // namespace annotation
