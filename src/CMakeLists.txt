
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
