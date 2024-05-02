#include <utils/spdlog.h>

#include <algorithm>
#include <cstddef>
#include <map>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace ranges = std::ranges;
namespace views = std::ranges::views;


auto main() -> int
{
    spdlog::info("Hello World");

    return 0;
}
