find_package(Eigen3 REQUIRED)
add_executable(tokenizer)
target_sources(tokenizer
    PRIVATE
        main.cpp
)
target_link_libraries(tokenizer
    PRIVATE
        zikhron::spaced_repetition
        zikhron::annotation
        zikhron::warnings
        zikhron::utils
        spdlog::spdlog
        zikhron::utils
        fmt::fmt
        Boost.DI
)
set_property(TARGET tokenizer PROPERTY CXX_STANDARD 23)
set_target_properties(tokenizer
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
