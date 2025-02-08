// #include <spdlog/spdlog.h>
//
// #include <cstdlib>
// #include <iostream>
//
// auto main(int argc, char* argv[]) -> int
// {
//     std::string test;
//     std::cin >> test;
//     std::cout << "Got: " << test << " :: \n";
//     if (const char* env_p = std::getenv("JAVA_HOME")) {
//         std::cerr << "Your PATH is: " << env_p << '\n';
//     }
//
//     for (int i = 0; i < argc; i++) {
//         spdlog::info("Arg: {}", argv[i]);
//     }
// }
#include <chrono>
#include <ctime>
#include <iostream>
#include <thread>

int main()
{
    using namespace std::chrono_literals;

    const std::time_t t = std::time(nullptr); // usually has "1 second" precision

    const std::chrono::time_point from = std::chrono::system_clock::from_time_t(t);

    std::this_thread::sleep_for(500ms);

    const auto diff = std::chrono::system_clock::now() - from;

    std::cout << diff << " ("
              << std::chrono::round<std::chrono::milliseconds>(diff)
              << ")\n";

    using Minutes = std::chrono::duration<double, std::ratio<60>>;
    using Days = std::chrono::duration<double, std::ratio<86400>>;
    const auto& minutes = std::chrono::duration_cast<Minutes>(diff);
    std::cout << "Minutes: " << minutes.count() << std::endl;
}
