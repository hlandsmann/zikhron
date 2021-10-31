add_library(dictionary OBJECT)
add_library(dictionary::dictionary ALIAS dictionary)
target_include_directories(dictionary
    INTERFACE
        include
    PRIVATE
        include/dictionary
)
target_sources(dictionary
    PRIVATE
        src/ZH_Dictionary.cpp
    PUBLIC
        include/dictionary/ZH_Dictionary.h
)
target_compile_features(dictionary PUBLIC cxx_std_20)
