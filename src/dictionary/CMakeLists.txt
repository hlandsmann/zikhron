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
        include/dictionary/Entry.h
        include/dictionary/Kana.h
        include/dictionary/Dictionary.h
        include/dictionary/DictionaryChi.h
        include/dictionary/DictionaryJpn.h
        src/Entry.cpp
        src/Kana.cpp
        src/DictionaryChi.cpp
        src/DictionaryJpn.cpp
)
set_property(TARGET zikhron_dictionary PROPERTY CXX_STANDARD 23)
target_link_libraries(zikhron_dictionary
    PUBLIC
        magic_enum::magic_enum
    PRIVATE
        pugixml::pugixml
        zikhron::miscellaneous
        zikhron::spaced_repetition
        zikhron::utils
        zikhron::warnings
)
