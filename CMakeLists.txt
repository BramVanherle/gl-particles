cmake_minimum_required(VERSION 3.20)
project(particles)

set(CMAKE_CXX_STANDARD 17)

# Source files
set(SOURCES
        src/main.cpp
        external/glad/src/glad.c
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/backends/imgui_impl_opengl3.cpp
        src/Particle.h
        src/SimulationParams.h
        src/ComputeShader.h
        src/RenderShader.h
)

add_executable(particles ${SOURCES})

# Include directories
target_include_directories(particles PRIVATE
        src/
        external/glad/include
        external/glfw/include
        external/glm
        external/imgui
        external/imgui/backends
)

# Link GLFW
target_link_libraries(particles PRIVATE
        ${CMAKE_SOURCE_DIR}/external/glfw/lib-mingw-w64/libglfw3.a
        opengl32
        gdi32
        user32
        shell32
)

# Copy GLFW dll to output
add_custom_command(TARGET particles POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/external/glfw/lib-mingw-w64/glfw3.dll"
        "$<TARGET_FILE_DIR:particles>"
)