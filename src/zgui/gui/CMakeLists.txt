add_library(zgui_gui)
add_library(zgui::gui ALIAS zgui_gui)

target_sources(zgui_gui
    PRIVATE
        include/gui/DisplayAnnotation.h
        include/gui/DisplayText.h
        include/gui/DisplayVideo.h
        include/gui/DisplayVocables.h
        include/gui/FileDialog.h
        include/gui/GlWindow.h
        include/gui/GroupAdd.h
        include/gui/GroupVideo.h
        include/gui/MainWindow.h
        include/gui/TabCard.h
        include/gui/TabVideo.h
        include/gui/TokenizationOverlay.h
        include/gui/VocableOverlay.h
        include/gui/theme/Sizes.h
        src/DisplayAnnotation.cpp
        src/DisplayText.cpp
        src/DisplayVideo.cpp
        src/DisplayVocables.cpp
        src/FileDialog.cpp
        src/GlWindow.cpp
        src/GroupAdd.cpp
        src/GroupVideo.cpp
        src/MainWindow.cpp
        src/TabCard.cpp
        src/TabVideo.cpp
        src/TokenizationOverlay.cpp
        src/VocableOverlay.cpp
)

target_include_directories(zgui_gui
    PRIVATE
        include/gui
    INTERFACE
        include
)

target_link_libraries(zgui_gui
    PUBLIC
        Pal::Sigslot
    PRIVATE
        ${OPENGL_LIBRARIES}
        # epoxy
        glfw
        imgui::imgui
        spdlog::spdlog
        zgui::context
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
set_property(TARGET zgui_gui PROPERTY CXX_STANDARD 23)
