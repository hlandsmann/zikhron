cmake_minimum_required(VERSION 3.5)

project(ImGuiFileDialog)

add_library(ImGuiFileDialog)
add_library(ImGui::FileDialog ALIAS ImGuiFileDialog)

target_sources(ImGuiFileDialog
    PRIVATE
        src/ImGuiFileDialog.cpp
        include/ImGuiFileDialog/ImGuiFileDialog.h
        include/ImGuiFileDialog/ImGuiFileDialogConfig.h
)

target_include_directories(ImGuiFileDialog 
    PRIVATE
        include/ImGuiFileDialog
    INTERFACE
        include
)

target_link_libraries(ImGuiFileDialog
    PUBLIC
        imgui::imgui
        stb::image
)

if(UNIX)
    target_compile_options(ImGuiFileDialog PUBLIC -Wno-unknown-pragmas)
endif()
