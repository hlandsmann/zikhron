add_library(zikhron_utils)
add_library(zikhron::utils ALIAS zikhron_utils)
target_include_directories(zikhron_utils
    INTERFACE
        include
    PRIVATE
        include/utils
)
target_sources(zikhron_utils
    PRIVATE
        include/utils/Algorithm.h
        include/utils/Crc32.h
        include/utils/Memoizer.h
        include/utils/Memory.h
        include/utils/StringU8.h
        include/utils/Time.h
        include/utils/Variant.h
        include/utils/counting_iterator.h
        include/utils/format.h
        include/utils/index_map.h
        include/utils/min_element_val.h
        include/utils/string_split.h
        src/Crc32.cpp
        src/StringU8.cpp
        src/Time.cpp
        src/string_split.cpp
)
pkg_search_module(ICU_UC REQUIRED icu-uc)
pkg_search_module(ICU_IO REQUIRED icu-io)

target_link_libraries(zikhron_utils
    PUBLIC
        ${ICU_UC_LIBRARIES}
        ${ICU_IO_LIBRARIES}
        spdlog::spdlog
    PRIVATE
        zikhron::warnings
)

set_property(TARGET zikhron_utils PROPERTY CXX_STANDARD 23)
