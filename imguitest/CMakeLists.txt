find_package(OpenGL REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/gen_cfg_json.cmake)
gen_cfg_json()
add_executable(imguitest)

target_sources(imguitest
    PRIVATE
        main.cpp
        zgui/Fonts.cpp
        zgui/Fonts.h
        zgui/MainWindow.cpp
        zgui/MainWindow.h
        zgui/MediaPlayer.cpp
        zgui/MediaPlayer.h
        zgui/SideBar.cpp
        zgui/SideBar.h
        zgui/TextDraw.cpp
        zgui/TextDraw.h
        zgui/Theme.cpp
        zgui/Theme.h
        zgui/widget/Button.cpp
        zgui/widget/Button.h
        zgui/widget/ImageButton.cpp
        zgui/widget/ImageButton.h
        zgui/widget/Layout.cpp
        zgui/widget/Layout.h
        zgui/widget/Widget.cpp
        zgui/widget/Widget.h
        zgui/widget/Window.cpp
        zgui/widget/Window.h
        zgui/widget/imglog.cpp
        zgui/widget/imglog.h
)

target_link_libraries(imguitest
    PRIVATE
        ${OPENGL_LIBRARIES}
        glfw
        mpv
        epoxy
        imgui::imgui
        spdlog::spdlog
        folly
        # zikhron::multimedia
        # zikhron::annotation
        zikhron::dictionary
        # zikhron::miscellaneous
        # zikhron::spaced_repetition
        # zikhron::warnings
)

target_include_directories(imguitest
    PRIVATE
        zgui
)

set_property(TARGET imguitest PROPERTY CXX_STANDARD 23)
