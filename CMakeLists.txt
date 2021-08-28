cmake_minimum_required(VERSION 3.19)
project(zikhron)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FindPkgConfig)

set(CMAKE_AUTOMOC ON)
find_package(Qt5 REQUIRED Qml Quick Gui QuickControls2)
qt5_add_resources(qml_QRC src/QML/qml.qrc)
set(QML_LIBRARIES Qt5::Qml Qt5::Quick Qt5::QuickControls2)

add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/cppcoro)
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/fmt)

pkg_search_module(ICU_UC REQUIRED icu-uc)
pkg_search_module(ICU_IO REQUIRED icu-io)
set(ICU_LIBRARIES "${ICU_UC_LIBRARIES}" "${ICU_IO_LIBRARIES}")

set(PROJECT_INCLUDES ${PROJECT_SOURCE_DIR}/src)
set(SOURCE src/Application.cpp
           src/TextCard.cpp
           src/VocabularySR.cpp
           src/VocabularySR_misc.cpp
           src/VocabularySR_TreeWalker.cpp
           src/ZH_Dictionary.cpp
           src/ZH_Annotator.cpp
           src/DataThread.cpp
           src/utils/StringU8.cpp
           src/utils/Markup.cpp
           src/gui/card/Annotate.cpp
           src/gui/card/Display.cpp
           src/gui/card/Edit.cpp
)
add_executable(${PROJECT_NAME} ${SOURCE} ${qml_QRC})
target_link_libraries(${PROJECT_NAME}
                      PRIVATE
                        ${ICU_LIBRARIES}
                        ${QML_LIBRARIES}
                        fmt::fmt
                        gsl tcmalloc profiler
                        pthread
                        cppcoro
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_options(${PROJECT_NAME} PRIVATE LINKER:--gc-sections,--relax)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDES})
