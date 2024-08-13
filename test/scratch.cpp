#include <utils/format.h>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <map>
#include <memory>
#include <ranges>
#include <sigslot/signal.hpp>
#include <string>
#include <utility>
#include <vector>

namespace ranges = std::ranges;
namespace views = std::ranges::views;
using namespace std::literals;

struct ClassA
{
    void doSomething()
    {
        spdlog::info("DoSomething");
        sig_string("DoSomething");
    }

    sigslot::signal<std::string> sig_string;
    std::string name = "ClassA";
};

struct ClassB
{
    void rcv_str(const std::string& text)
    {
        spdlog::info("rec from {}, text: {}", name, text);
    }

    std::string name = "ClassB";
};

auto main() -> int
{
    // spdlog::info("Hello World");
    //
    // ClassA classA;
    // classA.doSomething();
    //
    // {
    //     auto classB = std::make_shared<ClassB>();
    //     classA.sig_string.connect(&ClassB::rcv_str, classB);
    //     classA.doSomething();
    // }
    // classA.doSomething();
    //
    // for (const char c : "NSAR"s) {
    //     spdlog::info("{} : {:x}", c, c);
    // }

    auto vec1 = std::vector<int>{9, 1, 2, 3, 4, 0, 3, 2, 2, 1};
    auto vec2 = std::vector<int>{9, 1, 2, 3, 4, 0, 3, 2, 2, 1};
    auto it1 = std::next(vec1.begin(), 4);
    spdlog::info("it1: {}", *it1);
    auto rit1 = std::reverse_iterator{it1};
    spdlog::info("rit1: {}", *rit1);
    auto fit = std::find(rit1, std::rend(vec1), 9);
    spdlog::info("fit1: {}", *fit);
    auto distance = static_cast<std::size_t>(std::distance(vec1.rbegin(), fit));
    spdlog::info("index: {}", vec1.size() - distance - 1);
    auto fit2 = std::next(vec2.rbegin(), distance);
    spdlog::info("fit2: {}", *fit2);
    if (fit2 == vec2.rend()) {
        spdlog::info("is rend");
    }

    return 0;
}
