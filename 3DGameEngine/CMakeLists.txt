cmake_minimum_required(VERSION 3.20)
project(Protoplay)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# GLFW from source
add_subdirectory(vendor/glfw)

# GLAD
add_library(glad STATIC vendor/glad/src/glad.c)
target_include_directories(glad PUBLIC vendor/glad/include)

# ImGui
add_library(imgui STATIC
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_impl_glfw.cpp
    vendor/imgui/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC vendor/imgui)
target_link_libraries(imgui glfw glad)

# Game DLL
add_library(game SHARED
    src/game.cpp
    src/movement_system.cpp
    src/collision_system.cpp
)
target_include_directories(game PRIVATE
    src
    vendor/glm-master
    vendor/glad/include
)
target_link_libraries(game glfw)
set_target_properties(game PROPERTIES PREFIX "")

# Engine EXE
add_executable(engine
    src/application.cpp
    src/ecs.cpp
    src/asset_manager.cpp
    src/render_system.cpp
    src/window.cpp
    src/events.cpp
    src/camera.cpp
    src/editor.cpp
    src/text.cpp
    src/serialization.cpp
    vendor/stb/stb_setup.cpp
)
target_include_directories(engine PRIVATE
    src
    vendor/glm-master
    vendor/glad/include
    vendor/imgui
    vendor/stb
)
target_link_libraries(engine glfw glad imgui opengl32)

add_dependencies(engine game)

# Copy resources directly next to exe
add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:engine>
)

# Copy shaders directly next to exe
file(GLOB SHADER_FILES src/*.vs src/*.fs)
add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${SHADER_FILES} $<TARGET_FILE_DIR:engine>
)

# Mesh Converter
add_executable(mesh_converter
    MeshBinaryConverter/mesh_converter.cpp
    MeshBinaryConverter/tiny_obj_loader_implementation.cpp
)
target_include_directories(mesh_converter PRIVATE
    MeshBinaryConverter
)
set_target_properties(mesh_converter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)