#include <utils/format.h>

#include <algorithm>
#include <cstddef>
#include <map>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace ranges = std::ranges;
namespace views = std::ranges::views;

auto test() -> const std::vector<int>&
{
    static std::vector<int> res;
    return res;
}

auto main() -> int
{
    spdlog::info("Hello World");
    auto res = test();

    return 0;
}
