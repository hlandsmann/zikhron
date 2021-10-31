add_library(annotation STATIC)
add_library(annotation::annotation ALIAS annotation)
target_include_directories(annotation
PRIVATE
    include/annotation
    src
PUBLIC
    include
)
target_sources(annotation
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
target_link_libraries(annotation
    PUBLIC
        dictionary::dictionary
    PRIVATE
        utils::utils
        spdlog::spdlog
        ctre::single-header
)

target_compile_features(annotation PUBLIC cxx_std_20)
