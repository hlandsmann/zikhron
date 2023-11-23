find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-4.0)

add_library(zikhron_gtkgui OBJECT)
add_library(zikhron::gtkgui ALIAS zikhron_gtkgui)

target_sources(zikhron_gtkgui
    PRIVATE
        src/AnnotationOverlay.cpp
        src/AnnotationOverlay.h
        src/Application.cpp
        src/ButtonGroup.cpp
        src/ButtonGroup.h
        src/CardDraw.cpp
        src/CardDraw.h
        src/DataThread.cpp
        src/DataThread.h
        src/DisplayCard.cpp
        src/DisplayCard.h
        src/GTKMM_includes.h
        src/MainWindow.cpp
        src/MainWindow.h
        src/MediaButtons.cpp
        src/MediaButtons.h
        src/MediaSlider.cpp
        src/MediaSlider.h
        src/NotebookPage.h
        src/SubtitleComboBox.cpp
        src/SubtitleComboBox.h
        src/SubtitleOverlay.cpp
        src/SubtitleOverlay.h
        src/SupplyAudio.cpp
        src/SupplyAudio.h
        src/TextDraw.cpp
        src/TextDraw.h
        src/VideoSpace.cpp
        src/VideoSpace.h
        src/VocableList.cpp
        src/VocableList.h
        src/VocableOverlay.cpp
        src/VocableOverlay.h
    PUBLIC
        include/gtkgui/Application.h
)
target_include_directories(zikhron_gtkgui
    PRIVATE
        ${GTKMM_INCLUDE_DIRS}
        src
        include/gtkgui
    INTERFACE
        include
)

target_link_libraries(zikhron_gtkgui
    PUBLIC
        nlohmann_json::nlohmann_json
    PRIVATE
        ${GTKMM_LIBRARIES}
        spdlog::spdlog
        zikhron::annotation
        zikhron::dictionary
        zikhron::multimedia
        zikhron::spaced_repetition
        zikhron::warnings
)

set_property(TARGET zikhron_gtkgui PROPERTY CXX_STANDARD 23)
