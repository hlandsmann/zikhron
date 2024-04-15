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
        include/annotation/Card.h
        include/annotation/CardDB.h
        include/annotation/Ease.h
        include/annotation/JieBa.h
        include/annotation/Token.h
        include/annotation/TokenText.h
        include/annotation/Word.h
        include/annotation/WordDB.h
        include/annotation/ZH_Tokenizer.h
        src/Card.cpp
        src/CardDB.cpp
        src/Ease.cpp
        src/JieBa.cpp
        src/Token.cpp
        src/TokenText.cpp
        src/Word.cpp
        src/WordDB.cpp
        src/ZH_Tokenizer.cpp
)
target_link_libraries(zikhron_annotation
    PUBLIC
        zikhron::dictionary
        zikhron::miscellaneous
    PRIVATE
        Microsoft.GSL::GSL
        ctre::ctre
        cppjieba::cppjieba
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        zikhron::spaced_repetition
        zikhron::utils
        zikhron::warnings
)

set_property(TARGET zikhron_annotation PROPERTY CXX_STANDARD 23)
