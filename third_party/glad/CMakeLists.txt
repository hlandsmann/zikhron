add_library(glad)
add_library(glad::glad ALIAS glad)
target_sources(glad
    PRIVATE
        include/KHR/khrplatform.h
        include/glad/glad.h
        src/glad.c
)
target_include_directories(glad
    PUBLIC
        include
)
target_link_libraries(glad
    PUBLIC
        glfw
)

