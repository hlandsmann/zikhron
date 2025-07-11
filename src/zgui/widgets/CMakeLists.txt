add_library(zgui_widgets)
add_library(zgui::widgets ALIAS zgui_widgets)

target_sources(zgui_widgets
    PRIVATE
        include/widgets/Box.h
        include/widgets/Button.h
        include/widgets/Child.h
        include/widgets/Grid.h
        include/widgets/Image.h
        include/widgets/ImageButton.h
        include/widgets/Label.h
        include/widgets/Layer.h
        include/widgets/MediaSlider.h
        include/widgets/Overlay.h
        include/widgets/ScrollArea.h
        include/widgets/Separator.h
        include/widgets/SteppedSlider.h
        include/widgets/TextToken.h
        include/widgets/TextTokenSeq.h
        include/widgets/ToggleButtonGroup.h
        include/widgets/Video.h
        include/widgets/Window.h
        include/widgets/detail/MetaBox.h
        include/widgets/detail/Widget.h
        src/Box.cpp
        src/Button.cpp
        src/Child.cpp
        src/Grid.cpp
        src/Image.cpp
        src/ImageButton.cpp
        src/Label.cpp
        src/Layer.cpp
        src/MediaSlider.cpp
        src/MetaBox.cpp
        src/Overlay.cpp
        src/ScrollArea.cpp
        src/Separator.cpp
        src/SteppedSlider.cpp
        src/TextToken.cpp
        src/TextTokenSeq.cpp
        src/ToggleButtonGroup.cpp
        src/Video.cpp
        src/Widget.cpp
        src/Window.cpp
)

target_include_directories(zgui_widgets
    PRIVATE
        include/widgets
    INTERFACE
        include
)

target_link_libraries(zgui_widgets
    PUBLIC
        imgui::imgui
        ImGui::FileDialog
        magic_enum::magic_enum
        zgui::context
        zikhron::annotation
    PRIVATE
        ${OPENGL_LIBRARIES}
        glad::glad
        glfw
        spdlog::spdlog
        zikhron::kocoro
        zikhron::database
        zikhron::multimedia
        zikhron::utils
        zikhron::warnings
)

set_property(TARGET zgui_widgets PROPERTY CXX_STANDARD 23)
