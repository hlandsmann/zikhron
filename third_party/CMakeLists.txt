add_library(imgui)
add_library(imgui::imgui ALIAS imgui)
target_sources(imgui
    PRIVATE
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_opengl3.cpp
        imgui/imgui.cpp
        imgui/imgui.h
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_internal.h
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/imstb_rectpack.h
        imgui/imstb_textedit.h
        imgui/imstb_truetype.h
)
target_include_directories(imgui
    PUBLIC
        imgui
)
target_link_libraries(imgui
    PUBLIC
        glfw
)
target_compile_definitions(imgui PUBLIC IMGUI_DEFINE_MATH_OPERATORS)

add_library(stb_image)
add_library(stb::image ALIAS stb_image)
target_sources(stb_image
    PRIVATE
        stb/include/stb/stb_image.h
        stb/src/stb_image.c
)
target_include_directories(stb_image
    INTERFACE
        stb/include
    PRIVATE
        stb/include/stb
)

add_subdirectory(cppjieba)
add_subdirectory(glad)
