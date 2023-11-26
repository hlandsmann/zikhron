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
        include/utils/Memoizer.h
        include/utils/Property.h
        include/utils/StringU8.h
        include/utils/counting_iterator.h
        include/utils/index_map.h
        include/utils/min_element_val.h
        include/utils/variant_cast.h
        src/Property.cpp
        src/StringU8.cpp
)
pkg_search_module(ICU_UC REQUIRED icu-uc)
pkg_search_module(ICU_IO REQUIRED icu-io)

target_link_libraries(zikhron_utils
    PUBLIC
        ${ICU_UC_LIBRARIES}
        ${ICU_IO_LIBRARIES}
    PRIVATE
        zikhron::warnings
        spdlog::spdlog
)

set_property(TARGET zikhron_utils PROPERTY CXX_STANDARD 23)
