cmake_minimum_required(VERSION 3.16)
project(ProtoPlay VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "")

find_package(Lua REQUIRED)

include(FetchContent)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)

FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG        v3.3.0
)

FetchContent_MakeAvailable(glfw glm sol2)

set(GLAD_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/vendor/glad/include")
set(GLAD_SOURCES "vendor/glad/src/glad.c")

add_library(glad STATIC ${GLAD_SOURCES})
target_include_directories(glad PUBLIC ${GLAD_INCLUDE_DIR})

set(ENGINE_SOURCES
    application.cpp
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
    lua_interface.cpp
)

add_executable(${PROJECT_NAME} ${ENGINE_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${LUA_INCLUDE_DIR}
)


target_precompile_headers(${PROJECT_NAME} PRIVATE
    <vector>
    <string>
    <iostream>
    <memory>
    <algorithm>
    <map>
    <unordered_map>
    <glm/glm.hpp>
    <sol/sol.hpp>
    <glad/glad.h>
    <GLFW/glfw3.h>
)

find_package(OpenGL REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glad    
    glfw
    OpenGL::GL
    glm
    ${LUA_LIBRARIES}
    sol2
)


file(GLOB_RECURSE LUA_SCRIPTS "${CMAKE_SOURCE_DIR}/scripts/*.lua")
if (LUA_SCRIPTS)
    file(COPY ${LUA_SCRIPTS} DESTINATION "${CMAKE_BINARY_DIR}/scripts")
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

if (APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa" "-framework IOKit" "-framework CoreFoundation")
endif()
