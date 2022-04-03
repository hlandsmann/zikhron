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
        src/SR_DataBase.cpp
        src/VocabularySR_misc.cpp
        src/VocabularySR.cpp
    PUBLIC
        include/spaced_repetition/VocabularySR.h
        include/spaced_repetition/SR_DataBase.h
)
target_link_libraries(zikhron_spaced_repetition
    PUBLIC
        zikhron::annotation
        zikhron::dictionary
        zikhron::utils
        nlohmann_json::nlohmann_json
    PRIVATE
        cppcoro
        fmt::fmt
        spdlog::spdlog
        zikhron::warnings
)

target_compile_features(zikhron_spaced_repetition PUBLIC cxx_std_20)
target_compile_definitions(zikhron_spaced_repetition PRIVATE spaced_repetition_internal_include=1)
