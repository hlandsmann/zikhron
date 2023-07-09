include(${CMAKE_SOURCE_DIR}/cmake/gen_cfg_json.cmake)
gen_cfg_json()

find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)

#test dictionary
add_executable(test_dictionary)
target_sources(test_dictionary
    PRIVATE
        dictionary/test.cpp
)
target_link_libraries(test_dictionary
    PRIVATE
        Catch2::Catch2
        Catch2::Catch2WithMain
        zikhron::dictionary
        zikhron::warnings
)
target_compile_definitions(test_dictionary PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)
catch_discover_tests(test_dictionary)

# #test treewalker
# add_executable(test_sr_treewalker)
# target_sources(test_sr_treewalker
#     PRIVATE
#         sr_treewalker/test.cpp
# )
# target_link_libraries(test_sr_treewalker
#     PRIVATE
#         Catch2::Catch2
#         Catch2::Catch2WithMain
#         zikhron::annotation
#         zikhron::dictionary
#         zikhron::spaced_repetition
#         zikhron::warnings
# )
# target_compile_definitions(test_sr_treewalker PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)
# catch_discover_tests(test_sr_treewalker)
add_subdirectory(sr_treewalker)

#test utl::linear_set
add_executable(test_utils_linear_set)
target_sources(test_utils_linear_set
    PRIVATE
        utils_linear_set/test.cpp
)
target_link_libraries(test_utils_linear_set
    PRIVATE
        Catch2::Catch2
        Catch2::Catch2WithMain
        zikhron::utils
        zikhron::warnings
        spdlog::spdlog
)
target_compile_definitions(test_utils_linear_set PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)
catch_discover_tests(test_utils_linear_set)
