
#include "EditorConfig.h"
#include "System/File.h"
#include "Math/Serialize.h"
#include "nlohmann_json/json.hpp"

#include <cstdio>

namespace
{
    constexpr const char* camera_position = "camera_position";
    constexpr const char* camera_viewport = "camera_viewport";
    constexpr const char* window_size = "window_size";
}

bool editor::SaveConfig(const char* config_file, const editor::Config& config)
{
    nlohmann::json json;

    json[camera_position] = config.camera_position;
    json[camera_viewport] = config.camera_viewport;
    json[window_size] = config.window_size;

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

    config.camera_position = json[camera_position];
    config.camera_viewport = json[camera_viewport];
    config.window_size = json[window_size];

    return true;
}
