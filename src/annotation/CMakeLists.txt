add_library(zikhron_annotation STATIC)
add_library(zikhron::annotation ALIAS zikhron_annotation)
target_include_directories(zikhron_annotation
PRIVATE
    include/annotation
    src
PUBLIC
    include
)
target_sources(zikhron_annotation
    PRIVATE
        src/Ease.cpp
        src/Markup.cpp
        src/TextCard.cpp
        src/ZH_Annotator.cpp
    PUBLIC
        include/annotation/Ease.h
        include/annotation/Markup.h
        include/annotation/ZH_Annotator.h
        include/annotation/TextCard.h
)
target_link_libraries(zikhron_annotation
    PUBLIC
        zikhron::dictionary
    PRIVATE
        Microsoft.GSL::GSL
        ctre::ctre
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        zikhron::utils
        zikhron::warnings
)

target_compile_features(zikhron_annotation PUBLIC cxx_std_20)
