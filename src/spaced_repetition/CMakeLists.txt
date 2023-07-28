add_library(zikhron_spaced_repetition)
add_library(zikhron::spaced_repetition ALIAS zikhron_spaced_repetition)
target_include_directories(zikhron_spaced_repetition
    PRIVATE
        include/spaced_repetition
    INTERFACE
        include
)
target_sources(zikhron_spaced_repetition
    PRIVATE
        include/spaced_repetition/Card.h
        include/spaced_repetition/DataBase_deprecated.h
        include/spaced_repetition/TreeWalker.h
        include/spaced_repetition/Vocable.h
        include/spaced_repetition/Vocabulary.h
        src/Card.cpp
        src/DataBase_deprecated.cpp
        src/Time.cpp
        src/Time.h
        src/TreeWalker.cpp
        src/Vocable.cpp
        src/Vocabulary.cpp
)
target_link_libraries(zikhron_spaced_repetition
    PUBLIC
        zikhron::annotation
        zikhron::dictionary
        zikhron::utils
        nlohmann_json::nlohmann_json
    PRIVATE
        fmt::fmt
        spdlog::spdlog
        zikhron::warnings
)

target_compile_features(zikhron_spaced_repetition PUBLIC cxx_std_20)
target_compile_definitions(zikhron_spaced_repetition PRIVATE spaced_repetition_internal_include=1)
