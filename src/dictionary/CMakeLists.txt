add_library(zikhron-dictionary)
add_library(zikhron::dictionary ALIAS zikhron-dictionary)
target_include_directories(zikhron-dictionary
    INTERFACE
        include
    PRIVATE
        include/dictionary
)
target_sources(zikhron-dictionary
    PRIVATE
        src/ZH_Dictionary.cpp
    PUBLIC
        include/dictionary/ZH_Dictionary.h
)
target_compile_features(zikhron-dictionary PUBLIC cxx_std_20)
target_link_libraries(zikhron-dictionary PRIVATE zikhron::warnings)
