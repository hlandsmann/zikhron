#pragma once
#include <dictionary/ZH_Dictionary.h>
#include <utils/StringU8.h>

#include <memory>
#include <string>

namespace annotation {

class Rules
{
public:
    Rules(std::shared_ptr<const ZH_Dictionary> dictionary);

    [[nodiscard]] auto findRule(const std::string& word) const -> std::string;

private:
    [[nodiscard]] auto AABB_rule(const utl::StringU8& u8Str) const -> std::string;
    [[nodiscard]] auto specialEnding_rule(const utl::StringU8& u8Str) const -> std::string;
    [[nodiscard]] auto longerWord_rule(const utl::StringU8& u8Str) const -> std::string;
    std::shared_ptr<const ZH_Dictionary> dictionary;
};

} // namespace annotation
