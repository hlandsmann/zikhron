add_library(zgui_context)
add_library(zgui::context ALIAS zgui_context)

target_sources(zgui_context
    PRIVATE
        include/context/Fonts.h
        include/context/GlfwImguiContext.h
        include/context/Texture.h
        include/context/Theme.h
        include/context/imglog.h
        src/stb_image.c
        src/Fonts.cpp
        src/GlfwImguiContext.cpp
        src/Texture.cpp
        src/Theme.cpp
        src/imglog.cpp
)

target_include_directories(zgui_context
    PRIVATE
        include/context
    INTERFACE
        include
)

target_link_libraries(zgui_context
    PRIVATE
        ${OPENGL_LIBRARIES}
        glfw
        imgui::imgui
        spdlog::spdlog
        stb::image
)
set_property(TARGET zgui_context PROPERTY CXX_STANDARD 23)
