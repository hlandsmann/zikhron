add_library(spaced_repetition OBJECT)
add_library(spaced_repetition::spaced_repetition ALIAS spaced_repetition)
target_include_directories(spaced_repetition
    PRIVATE
        include/spaced_repetition
    INTERFACE
        include
)
target_sources(spaced_repetition
    PRIVATE
        src/VocabularySR.cpp
        src/VocabularySR_misc.cpp
        src/VocabularySR_TreeWalker.cpp
        src/VocabularySR_X.cpp
    PUBLIC
        include/spaced_repetition/VocabularySR.h
)
target_link_libraries(spaced_repetition
    PUBLIC
        annotation::annotation
        dictionary::dictionary
        utils::utils
        fmt::fmt
        cppcoro
)

target_compile_features(spaced_repetition PUBLIC cxx_std_20)
