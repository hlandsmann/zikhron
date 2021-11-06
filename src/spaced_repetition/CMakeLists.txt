add_library(zikhron-spaced_repetition OBJECT)
add_library(zikhron::spaced_repetition ALIAS zikhron-spaced_repetition)
target_include_directories(zikhron-spaced_repetition
    PRIVATE
        include/spaced_repetition
    INTERFACE
        include
)
target_sources(zikhron-spaced_repetition
    PRIVATE
        src/VocabularySR.cpp
        src/VocabularySR_misc.cpp
        src/VocabularySR_TreeWalker.cpp
        src/VocabularySR_X.cpp
    PUBLIC
        include/spaced_repetition/VocabularySR.h
)
target_link_libraries(zikhron-spaced_repetition
    PUBLIC
        zikhron::annotation
        zikhron::dictionary
        zikhron::utils
        fmt::fmt
    PRIVATE
        zikhron::warnings
        cppcoro
)

target_compile_features(zikhron-spaced_repetition PUBLIC cxx_std_20)
