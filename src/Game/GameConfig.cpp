
#include "GameConfig.h"
#include "System/File.h"

#include <string>
#include "nlohmann/json.hpp"

bool game::LoadConfig(const char* config_file, game::Config& config)
{
    file::FilePtr file = file::OpenAsciiFile(config_file);
    if(!file)
        return false;

    const std::vector<byte> file_data = file::FileRead(file);
    const nlohmann::json& json = nlohmann::json::parse(file_data);

    config.use_port_range               = json.value("use_port_range", config.use_port_range);
    config.client_port                  = json.value("client_port", config.client_port);
    config.server_port                  = json.value("server_port", config.server_port);
    config.port_range_start             = json.value("port_range_start", config.port_range_start);
    config.port_range_end               = json.value("port_range_end", config.port_range_end);
    config.server_replication_interval  = json.value("server_replication_interval", config.server_replication_interval);
    config.client_time_offset           = json.value("client_time_offset", config.client_time_offset);

    config.application                  = json.value("application", config.application);
    config.organization                 = json.value("organization", config.organization);

    config.light_mask_texture           = json.value("light_mask_texture", config.light_mask_texture);
    config.sprite_shadow_texture        = json.value("sprite_shadow_texture", config.sprite_shadow_texture);

    return true;
}

void game::LoadUserConfig(const char* user_config_path, game::UserConfig& config)
{
    const std::string user_config_file = user_config_path + std::string("user_config.json");
    const bool file_exists = file::Exists(user_config_file.c_str());
    if(!file_exists)
        return;

    file::FilePtr file = file::OpenAsciiFile(user_config_file.c_str());
    if(!file)
        return;

    const std::vector<byte> file_data = file::FileRead(file);
    const nlohmann::json& json = nlohmann::json::parse(file_data);

    config.fullscreen = json.value("fullscreen", false);
    config.vsync = json.value("vsync", true);
}

void game::SaveUserConfig(const char* user_config_path, const game::UserConfig& config)
{
    nlohmann::json json;
    json["fullscreen"] = config.fullscreen;
    json["vsync"] = config.vsync;

    const std::string& serialized_config = json.dump(4);

    const std::string user_config_file = user_config_path + std::string("user_config.json");
    file::FilePtr file = file::CreateAsciiFile(user_config_file.c_str());
    std::fwrite(serialized_config.data(), serialized_config.length(), sizeof(char), file.get());
}
