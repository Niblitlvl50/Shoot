
########### CMake Setup ###########

cmake_minimum_required(VERSION 3.12.0)

project(shoot)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --debug -g -Wall -Wextra -Wno-unused-parameter")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")

# Disable debug checks for Chipmunk
add_compile_definitions(NDEBUG)


########### Projects ###########

# Mono
add_subdirectory("Mono1/")
include_directories("Mono1/src" "Mono1/third_party" "src/Shared")


# Shared lib
file(GLOB_RECURSE shared_source_files "src/Shared/*.cpp")
add_library(shared STATIC ${shared_source_files})


# Game lib
file(GLOB_RECURSE game_source_files "src/Game/*.cpp")
list(FILTER game_source_files EXCLUDE REGEX "main.cpp")
add_library(game_lib STATIC ${game_source_files})
target_include_directories(game_lib PRIVATE "src/Game")


# Editor exe
file(GLOB_RECURSE editor_source_files "src/Editor/*.cpp")
add_executable(editor MACOSX_BUNDLE ${editor_source_files})
add_dependencies(editor game_lib mono shared imgui)
target_include_directories(editor PRIVATE "src/Editor" "src/Game")
target_link_libraries(editor game_lib mono shared)


# Game exe
set(osx_bundle_icons osx/icons.icns)
set_source_files_properties(
    ${osx_bundle_icons} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)

# Get a list of all of the files in the res-folder
file(
    GLOB_RECURSE
    game_exe_resource_files
    "res/entities/*"
    "res/fonts/*"
    "res/paths/*"
    "res/sound/*"
    "res/sprites/*"
    "res/textures/*"
    "res/worlds/*"
    "res/game_config.json"
    "res/spawn_config.json"
    "res/sprite_atlas.png")

# Individually set the file's path properties
foreach(RES_FILE ${game_exe_resource_files})

    # Get the relative path from the res-folder to the particular file
    get_filename_component(RES_FILE_PATH ${RES_FILE} DIRECTORY)
    file(RELATIVE_PATH RES_PATH ${CMAKE_SOURCE_DIR} ${RES_FILE_PATH})

    # Set it's location inside the app package (under Resources)
    set_property(SOURCE ${RES_FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${RES_PATH}")

endforeach(RES_FILE)

add_executable(game_exe MACOSX_BUNDLE "src/Game/main.cpp" ${osx_bundle_icons} ${game_exe_resource_files})
add_dependencies(game_exe game_lib mono shared)
target_include_directories(game_exe PRIVATE "src/Game")
target_link_libraries(game_exe game_lib shared mono)

# set_target_properties(game_exe PROPERTIES
#     XCODE_GENERATE_SCHEME TRUE
#     XCODE_SCHEME_WORKING_DIRECTORY "."
# )

# Game test exe
file(GLOB_RECURSE game_test_source_files "src/tests/*.cpp")
add_executable(game_test_exe ${game_test_source_files})
add_dependencies(game_test_exe game_lib shared mono)
target_include_directories(game_test_exe PRIVATE "src/Game" "Mono1/third_party/gtest-1.7.0/include")
target_link_libraries(game_test_exe gtest game_lib shared mono)
