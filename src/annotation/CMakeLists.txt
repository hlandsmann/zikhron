add_library(zikhron_annotation)
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
        include/annotation/AdaptJiebaDict.h
        include/annotation/Ease.h
        include/annotation/FreqDictionary.h
        include/annotation/JieBa.h
        include/annotation/Rules.h
        include/annotation/Token.h
        include/annotation/TokenText.h
        include/annotation/Tokenizer.h
        src/AdaptJiebaDict.cpp
        src/Ease.cpp
        src/FreqDictionary.cpp
        src/JieBa.cpp
        src/Rules.cpp
        src/Token.cpp
        src/TokenText.cpp
        src/Tokenizer.cpp
)
target_link_libraries(zikhron_annotation
    PUBLIC
        # zikhron::database
        zikhron::dictionary
        zikhron::miscellaneous
    PRIVATE
        Microsoft.GSL::GSL
        cppjieba::cppjieba
        ctre::ctre
        magic_enum::magic_enum
        spdlog::spdlog
        zikhron::spaced_repetition
        zikhron::utils
        zikhron::warnings
)

set_property(TARGET zikhron_annotation PROPERTY CXX_STANDARD 23)
