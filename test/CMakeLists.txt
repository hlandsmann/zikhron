include(${CMAKE_SOURCE_DIR}/cmake/gen_cfg_json.cmake)
gen_cfg_json()
add_executable(sr_treewalker)
target_sources(sr_treewalker
    PRIVATE
        main.cpp
)
target_link_libraries(sr_treewalker
    PRIVATE
        spdlog::spdlog
        zikhron::annotation
        zikhron::dictionary
        zikhron::miscellaneous
        zikhron::spaced_repetition
        zikhron::warnings
)
set_property(TARGET sr_treewalker PROPERTY CXX_STANDARD 23)

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
