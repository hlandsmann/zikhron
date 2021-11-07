add_library(zikhron-annotation STATIC)
add_library(zikhron::annotation ALIAS zikhron-annotation)
target_include_directories(zikhron-annotation
PRIVATE
    include/annotation
    src
PUBLIC
    include
)
target_sources(zikhron-annotation
    PRIVATE
        src/Markup.cpp
        src/TextCard.cpp
        src/rapidjsonWrapper.h
        src/ZH_Annotator.cpp
    PUBLIC
        include/annotation/Ease.h
        include/annotation/Markup.h
        include/annotation/ZH_Annotator.h
        include/annotation/TextCard.h
)
target_link_libraries(zikhron-annotation
    PUBLIC
        zikhron::dictionary
    PRIVATE
        zikhron::utils
        zikhron::warnings
        spdlog::spdlog
        ctre::ctre
        Microsoft.GSL::GSL
)

target_compile_features(zikhron-annotation PUBLIC cxx_std_20)
