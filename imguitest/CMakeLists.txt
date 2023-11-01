find_package(OpenGL REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/gen_cfg_json.cmake)
gen_cfg_json()
add_executable(imguitest)
target_sources(imguitest
    PRIVATE
        main.cpp
        zgui/MainWindow.cpp
        zgui/MainWindow.h
        zgui/MediaPlayer.cpp
        zgui/MediaPlayer.h
        zgui/TextDraw.cpp
        zgui/TextDraw.h
        zgui/Texture.cpp
        zgui/Texture.h
)
target_link_libraries(imguitest
    PRIVATE
        ${OPENGL_LIBRARIES}
        glfw
        mpv
        epoxy
        imgui::imgui
        spdlog::spdlog
        # zikhron::multimedia
        # zikhron::annotation
        # zikhron::dictionary
        # zikhron::miscellaneous
        # zikhron::spaced_repetition
        # zikhron::warnings
)
target_include_directories(imguitest
    PRIVATE
        zgui
)
set_property(TARGET imguitest PROPERTY CXX_STANDARD 23)
