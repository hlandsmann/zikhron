add_library(utils OBJECT)
add_library(utils::utils ALIAS utils)
target_include_directories(utils
    INTERFACE
        include
    PRIVATE
        include/utils
)
target_sources(utils
    PRIVATE
        src/StringU8.cpp
    PUBLIC
        include/utils/Memoizer.h
        include/utils/min_element_val.h
        include/utils/StringU8.h
)
pkg_search_module(ICU_UC REQUIRED icu-uc)
pkg_search_module(ICU_IO REQUIRED icu-io)

target_link_libraries(utils
    PUBLIC
    ${ICU_UC_LIBRARIES}
    ${ICU_IO_LIBRARIES}
)

target_compile_features(utils PUBLIC cxx_std_20)
