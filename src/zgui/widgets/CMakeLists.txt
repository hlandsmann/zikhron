add_library(zgui_widgets)
add_library(zgui::widgets ALIAS zgui_widgets)

target_sources(zgui_widgets
    PRIVATE
        include/widgets/Box.h
        include/widgets/Button.h
        include/widgets/ImageButton.h
        include/widgets/TextToken.h
        include/widgets/TextTokenSeq.h
        include/widgets/ToggleButtonGroup.h
        include/widgets/Widget.h
        include/widgets/Window.h
        src/Box.cpp
        src/Button.cpp
        src/ImageButton.cpp
        src/TextToken.cpp
        src/TextTokenSeq.cpp
        src/ToggleButtonGroup.cpp
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
        fmt::fmt
        imgui::imgui
        magic_enum::magic_enum
        zgui::context
        zikhron::annotation
    PRIVATE
        zikhron::warnings
        zikhron::utils
        spdlog::spdlog
)

set_property(TARGET zgui_widgets PROPERTY CXX_STANDARD 23)
