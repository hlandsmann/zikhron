add_library(zikhron_kocoro)
add_library(zikhron::kocoro ALIAS zikhron_kocoro)

target_sources(zikhron_kocoro
    PRIVATE
       include/kocoro/Async.hpp
       include/kocoro/ScheduleEntry.hpp
       include/kocoro/Signal.hpp
       include/kocoro/SynchronousExecutor.hpp
       include/kocoro/Task.hpp
       include/kocoro/kocoro.hpp
       src/SynchronousExecutor.cpp
)

target_include_directories(zikhron_kocoro
    PRIVATE
        include/kocoro
    INTERFACE
        include
)

target_link_libraries(zikhron_kocoro
    PRIVATE
        zikhron::warnings
        spdlog::spdlog
)

set_property(TARGET zikhron_kocoro PROPERTY CXX_STANDARD 23)
