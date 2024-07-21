add_library(zikhron_miscellaneous)
add_library(zikhron::miscellaneous ALIAS zikhron_miscellaneous)

target_sources(zikhron_miscellaneous
    PRIVATE
        include/misc/Config.h
        include/misc/TokenizationChoice.h
        include/misc/Identifier.h
        src/Config.cpp
)

target_include_directories(zikhron_miscellaneous
    PRIVATE
        include/misc
        src
    INTERFACE
        include
)

target_link_libraries(zikhron_miscellaneous
    PUBLIC
        zikhron::utils
    PRIVATE
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        zikhron::warnings
)

set_property(TARGET zikhron_miscellaneous PROPERTY CXX_STANDARD 23)
