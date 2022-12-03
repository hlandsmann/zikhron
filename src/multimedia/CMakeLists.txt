find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-4.0)

add_library(zikhron_multimedia)
add_library(zikhron::multimedia ALIAS zikhron_multimedia)
target_sources(zikhron_multimedia
    PRIVATE
        include/multimedia/Subtitles.h
        include/multimedia/Videoplayer.h
        src/Subtitles.cpp
        src/Videoplayer.cpp
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
    PRIVATE
        ${GTKMM_LIBRARIES}
        Microsoft.GSL::GSL
        epoxy
        fmt::fmt
        spdlog::spdlog
        zikhron::utils
        zikhron::warnings
)
target_compile_features(zikhron_multimedia PRIVATE cxx_std_20)
