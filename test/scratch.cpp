#include <utils/format.h>

#include <algorithm>
#include <cstddef>
#include <map>
#include <memory>
#include <ranges>
#include <sigslot/signal.hpp>
#include <string>
#include <utility>
#include <vector>

namespace ranges = std::ranges;
namespace views = std::ranges::views;

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
    spdlog::info("Hello World");

    ClassA classA;
    classA.doSomething();

    {
        auto classB = std::make_shared<ClassB>();
        classA.sig_string.connect(&ClassB::rcv_str, classB);
        classA.doSomething();
    }
    classA.doSomething();

    return 0;
}
