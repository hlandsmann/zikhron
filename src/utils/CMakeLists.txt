add_library(zikhron-utils OBJECT)
add_library(zikhron::utils ALIAS zikhron-utils)
target_include_directories(zikhron-utils
    INTERFACE
        include
    PRIVATE
        include/utils
)
target_sources(zikhron-utils
    PRIVATE
        src/StringU8.cpp
    PUBLIC
        include/utils/Memoizer.h
        include/utils/min_element_val.h
        include/utils/StringU8.h
)
pkg_search_module(ICU_UC REQUIRED icu-uc)
pkg_search_module(ICU_IO REQUIRED icu-io)

target_link_libraries(zikhron-utils
    PUBLIC
        ${ICU_UC_LIBRARIES}
        ${ICU_IO_LIBRARIES}
    PRIVATE
        zikhron::warnings
)

target_compile_features(zikhron-utils PUBLIC cxx_std_20)
