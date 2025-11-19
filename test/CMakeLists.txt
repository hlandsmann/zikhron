include(${CMAKE_SOURCE_DIR}/cmake/gen_cfg_json.cmake)
gen_cfg_json()
add_subdirectory(scheduler)
add_subdirectory(tokenizer)
 
add_executable(sr_treewalker)
target_sources(sr_treewalker
    PRIVATE
        main.cpp
)
target_link_libraries(sr_treewalker
    PRIVATE
        Boost.DI
        spdlog::spdlog
        zikhron::database
        zikhron::annotation
        zikhron::dictionary
        zikhron::miscellaneous
        zikhron::spaced_repetition
        zikhron::warnings
)
set_property(TARGET sr_treewalker PROPERTY CXX_STANDARD 23)

add_executable(print_char_jpn)
target_sources(print_char_jpn
    PRIVATE
        print_char_jpn.cpp
)
target_link_libraries(print_char_jpn
    PRIVATE
        Boost.DI
        spdlog::spdlog
        zikhron::database
        zikhron::annotation
        zikhron::dictionary
        zikhron::miscellaneous
        zikhron::spaced_repetition
        zikhron::warnings
)
set_property(TARGET print_char_jpn PROPERTY CXX_STANDARD 23)

add_executable(kocoro_signal)
target_sources(kocoro_signal
    PRIVATE
        kocoro_signal.cpp
)
target_link_libraries(kocoro_signal
    PRIVATE
        zikhron::kocoro
        zikhron::warnings
        spdlog::spdlog
)
set_property(TARGET kocoro_signal PROPERTY CXX_STANDARD 23)

find_package(Eigen3 REQUIRED)
add_executable(schedulerTest)
target_sources(schedulerTest
    PRIVATE
        schedulerTest.cpp
)
target_link_libraries(schedulerTest
    PRIVATE
        zikhron::spaced_repetition
        zikhron::warnings
        zikhron::utils
        spdlog::spdlog
        Pal::Sigslot
        Eigen3::Eigen
)
set_property(TARGET schedulerTest PROPERTY CXX_STANDARD 23)

add_executable(scratch)
target_sources(scratch
    PRIVATE
        scratch.cpp
)
target_link_libraries(scratch
    PRIVATE
        Boost.DI
        zikhron::warnings
        zikhron::utils
        spdlog::spdlog
        Pal::Sigslot
)
set_property(TARGET scratch PROPERTY CXX_STANDARD 23)

add_executable(callee)
target_sources(callee
    PRIVATE
        callee.cpp
)
target_link_libraries(callee
    PRIVATE
        zikhron::warnings
        zikhron::utils
        spdlog::spdlog
        Pal::Sigslot
)
set_property(TARGET callee PROPERTY CXX_STANDARD 23)
