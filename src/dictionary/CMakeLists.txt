add_library(zikhron_dictionary STATIC)
add_library(zikhron::dictionary ALIAS zikhron_dictionary)
target_include_directories(zikhron_dictionary
    INTERFACE
        include
    PRIVATE
        include/dictionary
)
target_sources(zikhron_dictionary
    PRIVATE
        include/dictionary/ZH_Dictionary.h
        include/dictionary/Word.h
        include/dictionary/WordDB.h
        src/ZH_Dictionary.cpp
        src/Word.cpp
        src/WordDB.cpp
)
set_property(TARGET zikhron_dictionary PROPERTY CXX_STANDARD 23)
target_link_libraries(zikhron_dictionary
    PRIVATE
        zikhron::miscellaneous
        zikhron::spaced_repetition
        zikhron::utils
        zikhron::warnings
)
