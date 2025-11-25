cmake_minimum_required(VERSION 3.14)
project(ProtoPlay VERSION 1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "")

include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
    set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
endif()

# --- GLAD Configuration ---
set(GLAD_SOURCES "vendor/glad/src/glad.c")
set(GLAD_INCLUDE_DIR "vendor/glad/include")
include_directories(${GLAD_INCLUDE_DIR})

# --- GLFW FetchContent ---
include(FetchContent)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8 # Known stable tag
)
FetchContent_MakeAvailable(glfw)


FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1  # Use a known stable tag
)

FetchContent_MakeAvailable(glm)

# --- Engine Executable ---
set(ENGINE_SOURCES
    application.cpp
    ${GLAD_SOURCES}
    asset_manager.cpp
    camera.cpp
    collision_system.cpp
    ecs.cpp
    movement_system.cpp
    render_system.cpp
    stb_setup.cpp
    text.cpp
    tiny_obj_loader_implementation.cpp
    window.cpp
)

add_executable(${PROJECT_NAME} ${ENGINE_SOURCES})

find_package(OpenGL REQUIRED)

# --- Compile Definitions ---
target_compile_definitions(${PROJECT_NAME} PRIVATE
        PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

# --- Linking ---
target_link_libraries(${PROJECT_NAME}
    glfw
    OpenGL::GL
    glm
)

# --- Platform Specifics (macOS) ---
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa" "-framework IOKit" "-framework CoreFoundation")
endif()
