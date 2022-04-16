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
        src/Property.cpp
        src/StringU8.cpp
    PUBLIC
        include/utils/Memoizer.h
        include/utils/Property.h
        include/utils/StringU8.h
        include/utils/counting_iterator.h
        include/utils/linear_set.h
        include/utils/min_element_val.h
)
pkg_search_module(ICU_UC REQUIRED icu-uc)
pkg_search_module(ICU_IO REQUIRED icu-io)

target_link_libraries(zikhron_utils
    PUBLIC
        ${ICU_UC_LIBRARIES}
        ${ICU_IO_LIBRARIES}
    PRIVATE
        zikhron::warnings
)

target_compile_features(zikhron_utils PUBLIC cxx_std_20)
