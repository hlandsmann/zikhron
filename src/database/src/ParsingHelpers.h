#pragma once
#include <string_view>

namespace database {

void verifyFileType(std::string_view& rest, std::string_view type);
auto getValue(std::string_view& rest, std::string_view valueType) -> std::string_view;
auto getValueType(std::string_view rest) -> std::string_view;

} // namespace database
