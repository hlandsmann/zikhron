find_package(OpenGL REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/gen_cfg_json.cmake)
gen_cfg_json()
add_executable(imguitest)

target_sources(imguitest
    PRIVATE
        main.cpp
        zgui/Token.cpp
        zgui/Token.h
)

target_link_libraries(imguitest
    PRIVATE
        ${OPENGL_LIBRARIES}
        Boost.DI
        epoxy
        folly
        glfw
        imgui::imgui
        mpv
        spdlog::spdlog
        stb::image
        zgui::gui
        zgui::widgets
        zikhron::annotation
        zikhron::dictionary
        zikhron::miscellaneous
        zikhron::multimedia
        zikhron::spaced_repetition
        zikhron::warnings
)

target_include_directories(imguitest
    PRIVATE
        zgui
)

set_property(TARGET imguitest PROPERTY CXX_STANDARD 23)
