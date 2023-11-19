add_library(zgui_widgets)
add_library(zgui::widgets ALIAS zgui_widgets)

target_sources(zgui_widgets
    PRIVATE
        include/widgets/Box.h
        include/widgets/Button.h
        include/widgets/ImageButton.h
        include/widgets/TextDraw.h
        include/widgets/TokenText.h
        include/widgets/Widget.h
        include/widgets/Window.h
        src/Box.cpp
        src/Button.cpp
        src/ImageButton.cpp
        src/TextDraw.cpp
        src/TokenText.cpp
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
        zgui::context
    PRIVATE
        spdlog::spdlog
)

set_property(TARGET zgui_widgets PROPERTY CXX_STANDARD 23)
