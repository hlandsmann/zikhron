add_library(zikhron_dictionary)
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
        src/ZH_Dictionary.cpp
)
set_property(TARGET zikhron_dictionary PROPERTY CXX_STANDARD 23)
target_link_libraries(zikhron_dictionary
    PRIVATE
        zikhron::miscellaneous
        zikhron::spaced_repetition
        zikhron::utils
        zikhron::warnings
)
