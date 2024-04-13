
add_library(zikhron_markup STATIC)
add_library(zikhron::markup ALIAS zikhron_markup)
target_include_directories(zikhron_markup
PRIVATE
    include/markup
    src
PUBLIC
    include
)
target_sources(zikhron_markup
    PRIVATE
        include/markup/Markup.h
        include/markup/Word.h
        src/Markup.cpp
        src/Word.cpp
)
target_link_libraries(zikhron_markup
    PUBLIC
        zikhron::annotation
        zikhron::dictionary
        zikhron::miscellaneous
    PRIVATE
        Microsoft.GSL::GSL
        ctre::ctre
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        zikhron::utils
        zikhron::warnings
)

set_property(TARGET zikhron_markup PROPERTY CXX_STANDARD 23)
