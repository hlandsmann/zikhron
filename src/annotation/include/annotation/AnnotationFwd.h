#pragma once

#include <utils/StringU8.h>

#include <memory>
#include <vector>

namespace annotation {

using TokenizationChoice = std::vector<utl::StringU8>;
using TokenizationChoiceVec = std::vector<TokenizationChoice>;
} // namespace annotation
