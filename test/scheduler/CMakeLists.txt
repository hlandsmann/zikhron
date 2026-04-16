find_package(Eigen3 REQUIRED)
add_executable(scheduler)
target_sources(scheduler
    PRIVATE
        LSTM.cpp
        LSTM.h
        SRS_data.h
        main.cpp
        scheduler.cpp
        scheduler.h
        user.cpp
        user.h
        weight.cpp
        weight.h
        ThompsonWeight.cpp
        ThompsonWeight.h
        ThompsonScheduler.cpp
        ThompsonScheduler.h
)
target_link_libraries(scheduler
    PRIVATE
        Boost::math
        Eigen3::Eigen
        fmt::fmt
        spdlog::spdlog
        zikhron::spaced_repetition
        zikhron::utils
        zikhron::utils
        zikhron::warnings
)
set_property(TARGET scheduler PROPERTY CXX_STANDARD 23)
set_target_properties(scheduler
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
