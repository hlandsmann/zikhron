find_package(PkgConfig)
find_package(OpenGL REQUIRED)
pkg_check_modules(GTKMM gtkmm-4.0)

add_library(zikhron_multimedia)
add_library(zikhron::multimedia ALIAS zikhron_multimedia)
target_sources(zikhron_multimedia
    PRIVATE
        include/multimedia/ExtractSubtitles.h
        include/multimedia/MpvWrapper.h
        include/multimedia/Subtitle.h
        src/ExtractSubtitles.cpp
        src/MpvWrapper.cpp
)
target_include_directories(zikhron_multimedia
    INTERFACE
        include
    PRIVATE
        ${GTKMM_INCLUDE_DIRS}
        src
        include/multimedia
)
target_link_libraries(zikhron_multimedia
    PUBLIC
        avcodec
        avformat
        avutil
        mpv
        nlohmann_json::nlohmann_json
    PRIVATE
        ${OPENGL_LIBRARIES}
        ${GTKMM_LIBRARIES}
        magic_enum::magic_enum
        Microsoft.GSL::GSL
        ctre::ctre
        epoxy
        glfw
        spdlog::spdlog
        zikhron::kocoro
        zikhron::miscellaneous
        zikhron::utils
        zikhron::warnings
)
set_property(TARGET zikhron_multimedia PROPERTY CXX_STANDARD 23)
