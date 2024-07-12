#include "ParsingHelpers.h"

#include <utils/format.h>
#include <utils/string_split.h>

#include <stdexcept>
#include <string_view>

namespace database {

void verifyFileType(std::string_view& rest, std::string_view type)
{
    auto fileType = utl::split_front(rest, ';');
    if (fileType != type) {
        throw std::runtime_error(fmt::format("Invalid VideoPack. Failed to parse type: {}",
                                             fileType));
    }
}

auto getValue(std::string_view& rest, std::string_view valueType) -> std::string_view
{
    auto valueTypeSV = utl::split_front(rest, ':');
    if (valueTypeSV != valueType) {
        throw std::runtime_error(fmt::format("Expected \"{}\", got: {}", valueType, valueTypeSV));
    }
    return utl::split_front(rest, '\n');
}

auto getValueType(std::string_view rest) -> std::string_view
{
    return utl::split_front(rest, ':');
}
} // namespace database
