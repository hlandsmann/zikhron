find_package(Eigen3 REQUIRED)
add_executable(scheduler)
target_sources(scheduler
    PRIVATE
        main.cpp
        LSTM.cpp
        LSTM.h
        SRS_data.h
        user.cpp
        user.h
)
target_link_libraries(scheduler
    PRIVATE
        zikhron::spaced_repetition
        zikhron::warnings
        zikhron::utils
        spdlog::spdlog
        Eigen3::Eigen
)
set_property(TARGET scheduler PROPERTY CXX_STANDARD 23)
set_target_properties(scheduler
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
