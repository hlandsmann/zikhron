#pragma once
#include <utils/StringU8.h>
#include <vector>

namespace markup {
class paragraph {
public:
    paragraph() = default;

private:
    std::vector<utl::StringU8> words;
};
}  // namespace markup
