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
        src/ZH_Dictionary.cpp
    PUBLIC
        include/dictionary/ZH_Dictionary.h
)
target_compile_features(zikhron_dictionary PUBLIC cxx_std_20)
target_link_libraries(zikhron_dictionary PRIVATE zikhron::warnings)
