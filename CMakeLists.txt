
cmake_minimum_required(VERSION 3.12.0)

project(shoot)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    # 4100 "unreferenced formal parameter"
    # 4244 "conversion from '__int64' to 'uint32_t', possible loss of data"
    # 4245 "conversion from 'int' to 'uint32_t', signed/unsigned mismatch"
    # 4267 "conversion from 'size_t' to 'uint32_t', possible loss of data"
    # 4505 "unreferenced local function has been removed"
    add_compile_options("/W4" "/wd4100" "/wd4244" "/wd4245" "/wd4267" "/wd4505" "$<$<CONFIG:RELEASE>:/O2>")
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS=1)
else()
    add_compile_options("-Wall" "-Wextra" "-Wno-unused-parameter" "$<$<CONFIG:RELEASE>:-O3>")
endif()

# Disable debug checks for Chipmunk
add_compile_definitions(NDEBUG)


########### Projects ###########

# Mono
add_subdirectory("Mono1/")
include_directories("Mono1/src" "Mono1/third_party")


# Game lib
file(GLOB_RECURSE game_source_files "src/Game/*.cpp")
list(FILTER game_source_files EXCLUDE REGEX "main.cpp")
add_library(game_lib STATIC ${game_source_files})
target_include_directories(game_lib PRIVATE "src/Game")


# Editor exe
file(GLOB_RECURSE editor_source_files "src/Editor/*.cpp")
add_executable(editor ${editor_source_files})
add_dependencies(editor game_lib mono imgui)
target_include_directories(editor PRIVATE "src/Editor" "src/Game")
target_link_libraries(editor game_lib mono)


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
    "res/pickup_config.json"
    "res/region_config.json"
    "res/spawn_config.json"
    "res/weapon_config.json"
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
add_dependencies(game_exe game_lib mono)
target_include_directories(game_exe PRIVATE "src/Game")
target_link_libraries(game_exe game_lib mono)

# set_target_properties(game_exe PROPERTIES
#     XCODE_GENERATE_SCHEME TRUE
#     XCODE_SCHEME_WORKING_DIRECTORY "."
# )

# Game test exe
file(GLOB_RECURSE game_test_source_files "src/tests/*.cpp")
add_executable(game_test_exe ${game_test_source_files})
add_dependencies(game_test_exe game_lib mono)
target_include_directories(game_test_exe PRIVATE "src/Game" "Mono1/third_party/gtest-1.7.0/include")
target_compile_definitions(game_test_exe PRIVATE GTEST_HAS_TR1_TUPLE=0)
target_link_libraries(game_test_exe gtest game_lib mono)
