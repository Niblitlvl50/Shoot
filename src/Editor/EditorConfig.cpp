
#include "EditorConfig.h"
#include "System/File.h"
#include "Math/Serialize.h"
#include "Rendering/Serialize.h"
#include "nlohmann/json.hpp"

#include <cstdio>

namespace
{
    constexpr const char* camera_position = "camera_position";
    constexpr const char* camera_viewport = "camera_viewport";
    constexpr const char* window_position = "window_position";
    constexpr const char* window_size = "window_size";
    constexpr const char* draw_object_names = "draw_object_names";
    constexpr const char* draw_snappers = "draw_snappers";
    constexpr const char* background_color = "background_color";
    constexpr const char* active_panel_index_id = "active_panel_index";
}

bool editor::SaveConfig(const char* config_file, const editor::Config& config)
{
    nlohmann::json json;

    json[camera_position] = config.camera_position;
    json[camera_viewport] = config.camera_viewport;
    json[window_position] = config.window_position;
    json[window_size] = config.window_size;
    json[draw_object_names] = config.draw_object_names;
    json[draw_snappers] = config.draw_snappers;
    json[background_color] = config.background_color;
    json[active_panel_index_id] = config.active_panel_index;

    const std::string& serialized_config = json.dump(4);

    File::FilePtr file = File::CreateAsciiFile(config_file);
    std::fwrite(serialized_config.data(), serialized_config.length(), sizeof(char), file.get());

    return true;
}

bool editor::LoadConfig(const char* config_file, editor::Config& config)
{
    File::FilePtr file = File::OpenAsciiFile(config_file);
    if(!file)
        return false;

    std::vector<byte> file_data;
    File::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);

    if(json.count(camera_position) > 0)
        config.camera_position = json[camera_position];
    
    if(json.count(camera_viewport) > 0)
        config.camera_viewport = json[camera_viewport];

    if(json.count(window_position) > 0)
        config.window_position = json[window_position];
    
    if(json.count(window_size) > 0)
        config.window_size = json[window_size];
    
    if(json.count(draw_object_names) > 0)
        config.draw_object_names = json[draw_object_names];
    
    if(json.count(draw_snappers) > 0)
        config.draw_snappers = json[draw_snappers];

    if(json.count(background_color) > 0)
        config.background_color = json[background_color];

    if(json.count(active_panel_index_id) > 0)
        config.active_panel_index = json[active_panel_index_id];

    return true;
}
