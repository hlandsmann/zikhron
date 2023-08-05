find_package(benchmark REQUIRED)
add_executable(bench_utils_linear_set)
target_sources(bench_utils_linear_set
    PRIVATE
        utils_linear_set/bench.cpp
)
target_link_libraries(bench_utils_linear_set
    PRIVATE
        zikhron::utils
        benchmark::benchmark
        spdlog::spdlog
)
set_property(TARGET bench_utils_linear_set PROPERTY CXX_STANDARD 23)
