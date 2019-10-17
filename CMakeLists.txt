
cmake_minimum_required(VERSION 3.6)

project(shoot)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)

# Disable debug checks for Chipmunk
add_compile_definitions(NDEBUG)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter")

add_subdirectory("Mono1/")
include_directories("Mono1/src" "Mono1/third_party" "src/Shared")

# Shared lib
file(GLOB_RECURSE shared_source_files "src/Shared/*.cpp")
add_library(shared STATIC ${shared_source_files})

# Game lib
file(GLOB_RECURSE game_source_files "src/Game/*.cpp")
list(FILTER game_source_files EXCLUDE REGEX "main.cpp")
add_library(game STATIC ${game_source_files})
target_include_directories(game PRIVATE "src/Game")

# Editor exe
file(GLOB_RECURSE editor_source_files "src/Editor/*.cpp")
add_executable(editor ${editor_source_files})
add_dependencies(editor mono shared imgui)
target_include_directories(editor PRIVATE "src/Editor")
target_link_libraries(editor mono shared)


set(osx_bundle_icons osx/icons.icns)
set_source_files_properties(${osx_bundle_icons} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

# Game exe
file(GLOB_RECURSE game_exe_source_files "src/Game/main.cpp")
add_executable(game_exe MACOSX_BUNDLE ${osx_bundle_icons} ${game_exe_source_files})
add_dependencies(game_exe game mono shared)
target_link_libraries(game_exe game shared mono)

# Game test exe
file(GLOB_RECURSE game_test_source_files "src/tests/*.cpp")
add_executable(game_test_exe ${game_test_source_files})
add_dependencies(game_test_exe game shared mono)
target_include_directories(game_test_exe PRIVATE "src/game" "mono1/third_party/gtest-1.7.0/include")
target_link_libraries(game_test_exe gtest game shared mono)
