
#include "EditorConfig.h"
#include "System/File.h"
#include "Math/Serialize.h"
#include "Rendering/Serialize.h"
#include "nlohmann/json.hpp"

#include <cstdio>

namespace
{
    constexpr const char* camera_position_key = "camera_position";
    constexpr const char* camera_viewport_key = "camera_viewport";
    constexpr const char* window_position_key = "window_position";
    constexpr const char* window_size_key = "window_size";
    constexpr const char* draw_object_names_key = "draw_object_names";
    constexpr const char* draw_snappers_key = "draw_snappers";
    constexpr const char* draw_outline_key = "draw_outline";
    constexpr const char* draw_level_metadata_key = "draw_level_metadata";
    constexpr const char* snap_to_grid_key = "snap_to_grid";
    constexpr const char* grid_size_key = "grid_size";
    constexpr const char* selected_world_key = "selected_world";
}

bool editor::SaveConfig(const char* config_file, const editor::Config& config)
{
    nlohmann::json json;

    json[camera_position_key]       = config.camera_position;
    json[camera_viewport_key]       = config.camera_viewport;
    json[window_position_key]       = config.window_position;
    json[window_size_key]           = config.window_size;
    json[draw_object_names_key]     = config.draw_object_names;
    json[draw_snappers_key]         = config.draw_snappers;
    json[draw_outline_key]          = config.draw_outline;
    json[draw_level_metadata_key]   = config.draw_metadata;
    json[snap_to_grid_key]          = config.snap_to_grid;
    json[grid_size_key]             = config.grid_size;
    json[selected_world_key]        = config.selected_world;

    const std::string& serialized_config = json.dump(4);

    file::FilePtr file = file::CreateAsciiFile(config_file);
    std::fwrite(serialized_config.data(), serialized_config.length(), sizeof(char), file.get());

    return true;
}

bool editor::LoadConfig(const char* config_file, editor::Config& config)
{
    file::FilePtr file = file::OpenAsciiFile(config_file);
    if(!file)
        return false;

    const std::vector<byte> file_data = file::FileRead(file);
    const nlohmann::json& json = nlohmann::json::parse(file_data);

    config.camera_position      = json.value(camera_position_key, config.camera_position);
    config.camera_viewport      = json.value(camera_viewport_key, config.camera_viewport);
    config.window_position      = json.value(window_position_key, config.window_position);
    config.window_size          = json.value(window_size_key, config.window_size);
    config.draw_object_names    = json.value(draw_object_names_key, config.draw_object_names);
    config.draw_snappers        = json.value(draw_snappers_key, config.draw_snappers);
    config.draw_outline         = json.value(draw_outline_key, config.draw_outline);
    config.draw_metadata        = json.value(draw_level_metadata_key, config.draw_metadata);
    config.snap_to_grid         = json.value(snap_to_grid_key, config.snap_to_grid);
    config.grid_size            = json.value(grid_size_key, config.grid_size);
    config.selected_world       = json.value(selected_world_key, config.selected_world);

    return true;
}
