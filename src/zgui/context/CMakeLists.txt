add_library(zgui_context)
add_library(zgui::context ALIAS zgui_context)

target_sources(zgui_context
    PRIVATE
        include/context/ColorSet.h
        include/context/Drop.h
        include/context/Fonts.h
        include/context/GlfwImguiContext.h
        include/context/Texture.h
        include/context/Theme.h
        include/context/WidgetId.h
        include/context/WidgetState.h
        include/context/imglog.h
        src/ColorSet.cpp
        src/FontData.cpp
        src/FontData.h
        src/Fonts.cpp
        src/GlfwImguiContext.cpp
        src/Texture.cpp
        src/Theme.cpp
        src/WidgetId.cpp
        src/WidgetState.cpp
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
        glad::glad
        glfw
        imgui::imgui
        spdlog::spdlog
        stb::image
        zikhron::warnings
        zikhron::miscellaneous
)
set_property(TARGET zgui_context PROPERTY CXX_STANDARD 23)
