add_library(zikhron_miscellaneous OBJECT)
add_library(zikhron::miscellaneous ALIAS zikhron_miscellaneous)

target_sources(zikhron_miscellaneous
    PRIVATE
        src/Config.cpp
        include/misc/Config.h
)

target_include_directories(zikhron_miscellaneous
    PRIVATE
        src
        include/misc
    INTERFACE
        include
)

target_link_libraries(zikhron_miscellaneous
    PUBLIC
        nlohmann_json::nlohmann_json
    PRIVATE
        spdlog::spdlog
        zikhron::warnings
)

set_property(TARGET zikhron_miscellaneous PROPERTY CXX_STANDARD 23)
