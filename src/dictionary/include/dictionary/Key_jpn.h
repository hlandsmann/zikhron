#pragma once
#include <string>

namespace dictionary {

struct Key_jpn
{
    std::string key;
    std::string hint;
    std::string reading;
    auto operator<=>(const Key_jpn& other) const = default;
};

} // namespace dictionary
