find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-4.0)

add_library(zikhron_gtkgui OBJECT)
add_library(zikhron::gtkgui ALIAS zikhron_gtkgui)

target_sources(zikhron_gtkgui
    PRIVATE
        src/AnnotationOverlay.cpp
        src/AnnotationOverlay.h
        src/Application.cpp
        src/CardDraw.cpp
        src/CardDraw.h
        src/DataThread.cpp
        src/DataThread.h
        src/DisplayCard.cpp
        src/DisplayCard.h
        src/EaseChoice.cpp
        src/EaseChoice.h
        src/MainWindow.cpp
        src/MainWindow.h
        src/TextDraw.cpp
        src/TextDraw.h
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
    PUBLIC
        include
)

target_link_libraries(zikhron_gtkgui
    PRIVATE
        ${GTKMM_LIBRARIES}
        spdlog::spdlog
        zikhron::annotation
        zikhron::dictionary
        zikhron::spaced_repetition
        zikhron::warnings
)
