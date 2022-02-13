
add_library(zikhron-warnings INTERFACE)
target_compile_options(
    zikhron-warnings
    INTERFACE
        -Wall
        -Wextra
        -pedantic
)
add_library(zikhron::warnings ALIAS zikhron-warnings)

add_subdirectory(annotation)
add_subdirectory(dictionary)
add_subdirectory(gui)
add_subdirectory(spaced_repetition)
add_subdirectory(utils)

add_executable(zikhron)
set_target_properties(zikhron
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
target_sources(zikhron
    PRIVATE
        zikhron.cpp
)
target_link_libraries(zikhron
    PRIVATE
        zikhron::gui
)

target_compile_features(zikhron PRIVATE cxx_std_20)
target_link_options(zikhron PRIVATE LINKER:--gc-sections,--relax)
