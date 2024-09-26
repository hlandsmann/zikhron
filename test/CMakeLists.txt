include(${CMAKE_SOURCE_DIR}/cmake/gen_cfg_json.cmake)
gen_cfg_json()
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

add_executable(jumantest)
target_sources(jumantest
    PRIVATE
        # JpnDictionary.cpp
        # JpnDictionary.h
        # JpnTokenizer.cpp
        # JpnTokenizer.h
        # Jumanpp.cpp
        # Jumanpp.h
        jumantest.cpp
)
target_link_libraries(jumantest
    PRIVATE
        Boost.DI
        zikhron::dictionary
        zikhron::annotation
        spdlog::spdlog
        magic_enum::magic_enum
        jpp::jumandic
        pugixml::pugixml
)
set_property(TARGET jumantest PROPERTY CXX_STANDARD 23)

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

add_executable(scratch)
target_sources(scratch
    PRIVATE
        scratch.cpp
)
target_link_libraries(scratch
    PRIVATE
        zikhron::warnings
        zikhron::utils
        spdlog::spdlog
        Pal::Sigslot
)
set_property(TARGET scratch PROPERTY CXX_STANDARD 23)

