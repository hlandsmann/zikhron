#include <string_split.h>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <ios>
#include <string>
#include <string_view>
#include <utility>

namespace utl {
auto load_string_file(const std::filesystem::path& filename) -> std::string
{
    std::string result;
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.open(filename, std::ios_base::binary);
    auto sz = static_cast<std::size_t>(file_size(filename));
    result.resize(sz, '\0');
    file.read(result.data(), static_cast<std::streamsize>(sz));
    return result;
}

auto splitOnce(std::string_view str, char delim)
        -> std::pair<std::string_view, std::string_view>
{
    std::size_t found = str.find(delim);
    if (found == std::string_view::npos) {
        return {str, std::string_view()};
    }
    return {str.substr(0, found), str.substr(found + 1)};
}

auto split_front(std::string_view& sv, char delim) -> std::string_view
{
    std::size_t found = sv.find(delim);
    if (found == std::string_view::npos) {
        sv = std::string_view{};
        return sv;
    }
    auto result = sv.substr(0, found);
    sv = sv.substr(found + 1);
    return result;
}

auto extractSubstr(const std::string_view& str, char delimBegin, char delimEnd)
        -> std::pair<std::string_view, std::string_view>
{
    std::size_t foundBegin = str.find(delimBegin);
    std::size_t foundEnd = str.find(delimEnd);
    if (foundBegin == std::string_view::npos || foundEnd == std::string_view::npos || foundBegin > foundEnd) {
        return {std::string_view(), std::string_view()};
    }
    return {str.substr(foundBegin + 1, foundEnd - 1 - foundBegin), str.substr(foundEnd + 1)};
}
} // namespace utl
