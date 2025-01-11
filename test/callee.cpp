#include <spdlog/spdlog.h>

#include <cstdlib>
#include <iostream>

auto main(int argc, char* argv[]) -> int
{
    std::string test;
    std::cin >> test;
    std::cout << "Got: " << test << " :: \n";
    if (const char* env_p = std::getenv("JAVA_HOME")) {
        std::cerr << "Your PATH is: " << env_p << '\n';
    }

    for (int i = 0; i < argc; i++) {
        spdlog::info("Arg: {}", argv[i]);
    }
}
