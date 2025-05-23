add_library(zikhron-warnings INTERFACE)
target_compile_options(zikhron-warnings
    INTERFACE
        -Werror=non-virtual-dtor
        -Werror=return-type
        -Wall
        -Wextra
        -Wconversion
        -Wshadow
        -pedantic
        -fdiagnostics-color=always
        # -fsanitize=address
)

# target_compile_definitions(zikhron-warnings INTERFACE LSAN_OPTION=verbosity=1:log_threads=1

add_library(zikhron::warnings ALIAS zikhron-warnings)

add_subdirectory(annotation)
add_subdirectory(database)
add_subdirectory(dictionary)
add_subdirectory(kocoro)
add_subdirectory(misc)
add_subdirectory(multimedia)
add_subdirectory(spaced_repetition)
add_subdirectory(utils)
add_subdirectory(zgui)

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
        ${OPENGL_LIBRARIES}
        stdc++exp
        Boost.DI
        epoxy
        glfw
        imgui::imgui
        mpv
        spdlog::spdlog
        zgui::gui
        zgui::widgets
        zikhron::annotation
        zikhron::database
        zikhron::dictionary
        zikhron::kocoro
        zikhron::miscellaneous
        zikhron::multimedia
        zikhron::spaced_repetition
        zikhron::warnings
)

set_property(TARGET zikhron PROPERTY CXX_STANDARD 23)
target_link_options(zikhron PRIVATE LINKER:--gc-sections,--relax)
