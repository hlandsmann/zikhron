#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>

namespace utl {
auto load_string_file(const std::filesystem::path& filename) -> std::string;
auto splitOnce(std::string_view str, char delim) -> std::pair<std::string_view, std::string_view>;
auto split_front(std::string_view& sv, char delim) -> std::string_view;
auto extractSubstr(const std::string_view& str, char delimBegin, char delimEnd)
        -> std::pair<std::string_view, std::string_view>;
} // namespace utl
