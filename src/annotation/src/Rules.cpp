#include "Rules.h"

#include <dictionary/ZH_Dictionary.h>
#include <utils/StringU8.h>
#include <utils/spdlog.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace annotation {
Rules::Rules(std::shared_ptr<const ZH_Dictionary> _dictionary)
    : dictionary{std::move(_dictionary)}
{}

auto Rules::findRule(const std::string& word) const -> std::string
{
    auto u8Str = utl::StringU8{word};
    if (const auto result = AABB_rule(word); !result.empty()) {
        return result;
    }
    // if (const auto result = specialEnding_rule(word); !result.empty()) {
    //     return result;
    // }
    // if (const auto result = longerWord_rule(word); !result.empty()) {
    //     return result;
    // }

    return {};
}

auto Rules::approachRule(const std::string& word) const -> bool
{
    // ToDo: Implement this to support other rules also
    auto u8Str = utl::StringU8{word};
    return u8Str.length() <= 4;
}

auto Rules::AABB_rule(const utl::StringU8& u8Str) const -> std::string
{
    if (u8Str.length() != 4) {
        return {};
    }
    if (!(u8Str.at(0) == u8Str.at(1) && u8Str.at(2) == u8Str.at(3))) {
        return {};
    }
    auto result = u8Str.substr(1, 2);
    if (!dictionary->contains(result)) {
        return {};
    }
    return result;
}

auto Rules::specialEnding_rule(const utl::StringU8& u8Str) const -> std::string
{
    auto endings = std::vector<utl::CharU8>{{"了"}};

    if (ranges::none_of(endings,
                        [&u8Str](const utl::CharU8& charU8) -> bool {
                            return u8Str.back() == charU8;
                        })) {
        return {};
    }
    auto result = u8Str.substr(0, static_cast<long>(u8Str.length()) - 1);
    if (!dictionary->contains(result)) {
        return {};
    }
    return result;
}

auto Rules::longerWord_rule(const utl::StringU8& u8Str) const -> std::string
{
    if (u8Str.length() < 3) {
        return {};
    }
    const auto& word = u8Str.string();
    const auto span_lower = ZH_Dictionary::Lower_bound(word, dictionary->Simplified());
    const auto span_now = ZH_Dictionary::Upper_bound(word, span_lower);
    if (span_lower.empty()) {
        return {};
    }
    const auto it = ranges::find_if(span_lower,
                                    [&](const ZH_Dictionary::Key& key) -> bool {
                                        return key.key.starts_with(word);
                                    });
    if (it == span_lower.end()) {
        return {};
    }
    return it->key;
}
} // namespace annotation
