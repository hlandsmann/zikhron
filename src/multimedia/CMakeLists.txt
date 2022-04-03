find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-4.0)

add_library(zikhron_multimedia)
add_library(zikhron::multimedia ALIAS zikhron_multimedia)
target_sources(zikhron_multimedia
    PRIVATE
        include/multimedia/mediaplayer.h
        include/multimedia/subtitles.h
        src/mediaplayer.cpp
        src/subtitles.cpp
)
target_include_directories(zikhron_multimedia
    INTERFACE
        include
    PRIVATE
        src
        include/multimedia
)
target_link_libraries(zikhron_multimedia
    PUBLIC
        avcodec
        avformat
        avutil
        mpv
    PRIVATE
        ${GTKMM_LIBRARIES}
        epoxy
        fmt::fmt
        spdlog::spdlog
        zikhron::warnings
)