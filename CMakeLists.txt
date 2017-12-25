
cmake_minimum_required(VERSION 3.6)

project(shoot)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Wno-unused-parameter")

add_subdirectory("Mono1/")
include_directories("Mono1/MonoEngine" "Mono1/third_party" "src/Shared")

# Shared code
file(GLOB_RECURSE shared_source_files "src/Shared/*.cpp")
add_library(shared STATIC ${shared_source_files})

# Editor
file(GLOB_RECURSE editor_source_files "src/Editor/*.cpp")
add_executable(editor ${editor_source_files})
add_dependencies(editor mono shared imgui)
target_include_directories(editor PRIVATE "src/Editor")
target_link_libraries(editor mono shared)

# Game
file(GLOB_RECURSE game_source_files "src/Game/*.cpp")
add_executable(game ${game_source_files})
add_dependencies(game mono shared)
target_include_directories(game PRIVATE "src/Game")
target_link_libraries(game mono shared)

