
add_library(zgui_gui)
add_library(zgui::gui ALIAS zgui_gui)

target_sources(zgui_gui
    PRIVATE
        include/gui/CardDisplay.h
        include/gui/GlWindow.h
        include/gui/MainWindow.h
        include/gui/SideBar.h
        src/CardDisplay.cpp
        src/GlWindow.cpp
        src/MainWindow.cpp
        src/SideBar.cpp
)

target_include_directories(zgui_gui
    PRIVATE
        include/gui
    INTERFACE
        include
)

target_link_libraries(zgui_gui
    PRIVATE
        ${OPENGL_LIBRARIES}
        # epoxy
        folly
        glfw
        imgui::imgui
        spdlog::spdlog
        zgui::context
        zgui::widgets
        zikhron::multimedia
        zikhron::annotation
        zikhron::dictionary
        zikhron::miscellaneous
        zikhron::spaced_repetition
        zikhron::warnings
)
set_property(TARGET zgui_gui PROPERTY CXX_STANDARD 23)
