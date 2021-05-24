cmake_minimum_required(VERSION 3.5)
project(zikhron)

include(FindPkgConfig)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set(CMAKE_SYSTEM_NAME Generic)
# set(CMAKE_SYSTEM_VERSION 1)
# set(CMAKE_CXX_COMPILER g++)
# set(CMAKE_C_COMPILER gcc)
# #set(CMAKE_CXX_COMPILER g++-10.2.0)
# set(CMAKE_CXX_COMPILER clang++)
# set(CMAKE_C_COMPILER clang)

set(CXX_WARNINGLEVEL "-Wall -Wextra -Wshadow -Wno-class-memaccess")
set(CMAKE_CXX_FLAGS "-std=c++20 -pedantic -march=native -O2 -g3 ${CXX_WARNINGLEVEL}")
set(LINKER_FLAGS "-Wl,--gc-sections,--relax")

set(EXTERNAL_INCLUDES ${PROJECT_SOURCE_DIR}/third_party)
set(PROJECT_INCLUDES ${PROJECT_SOURCE_DIR}/src)
include_directories(${EXTERNAL_INCLUDES}
                    ${PROJECT_INCLUDES}

                    )

set(CMAKE_AUTOMOC ON)
find_package(Qt5 REQUIRED Qml Quick Gui QuickControls2)
qt5_add_resources(qml_QRC src/QML/qml.qrc)
set(QML_LIBRARIES Qt5::Qml Qt5::Quick Qt5::QuickControls2)

add_subdirectory(${EXTERNAL_INCLUDES}/fmt)
set_target_properties(fmt PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${EXTERNAL_INCLUDES}/fmt/include")

pkg_search_module(ICU_UC REQUIRED icu-uc)
pkg_search_module(ICU_IO REQUIRED icu-io)
set(ICU_LIBRARIES "${ICU_UC_LIBRARIES}" "${ICU_IO_LIBRARIES}")

set(SOURCE src/Application.cpp
           src/TextCard.cpp
           src/VocabularySR.cpp
           src/VocabularySR_misc.cpp
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
target_link_libraries(${PROJECT_NAME} ${ICU_LIBRARIES} ${QML_LIBRARIES} fmt::fmt ${LINKER_FLAGS})
