
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5 REQUIRED Qml Quick Gui QuickControls2)

add_library(gui SHARED)
add_library(gui::gui ALIAS gui)
target_include_directories(gui
    PRIVATE
        include/gui
        src
    INTERFACE
        include
)
qt5_add_resources(qml_QRC QML/qml.qrc)
target_sources(gui
    PRIVATE
        ${qml_QRC}
        src/Annotate.cpp
        src/Annotate.h
        src/Application.cpp
        src/DataThread.cpp
        src/DataThread.h
        src/Display.cpp
        src/Display.h
        src/Edit.cpp
        src/Edit.h
    PUBLIC
        include/gui/Application.h
)

target_link_libraries(gui
    PRIVATE
        Qt5::Qml
        Qt5::Quick
        Qt5::QuickControls2
        dictionary::dictionary
        spaced_repetition::spaced_repetition
        annotation::annotation
)
target_compile_features(gui PRIVATE cxx_std_20)
