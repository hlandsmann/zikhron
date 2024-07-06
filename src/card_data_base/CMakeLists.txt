add_library(zikhron_card_data_base STATIC)
add_library(zikhron::card_data_base ALIAS zikhron_card_data_base)
target_include_directories(zikhron_card_data_base
PRIVATE
    include/card_data_base
    src
PUBLIC
    include
)
target_sources(zikhron_card_data_base
    PRIVATE
        include/card_data_base/Card.h
        include/card_data_base/CardPack.h
        include/card_data_base/CardPackDB.h
        include/card_data_base/Video.h
        include/card_data_base/VideoPack.h
        include/card_data_base/VideoPackDB.h
        src/Card.cpp
        src/CardPack.cpp
        src/CardPackDB.cpp
        src/Video.cpp
        src/VideoPack.cpp
        src/VideoPackDB.cpp
)

set_property(TARGET zikhron_card_data_base PROPERTY CXX_STANDARD 23)
target_link_libraries(zikhron_card_data_base
    PRIVATE
        magic_enum::magic_enum
        spdlog::spdlog
        zikhron::annotation
        zikhron::dictionary
        zikhron::miscellaneous
        zikhron::warnings
)
