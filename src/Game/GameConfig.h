
#pragma once

#include <string>

namespace game
{
    struct Config
    {
        bool use_port_range = true;
        int client_port = 2100;
        int server_port = 2101;
        int port_range_start = 21000;
        int port_range_end = 22000;
        int server_replication_interval = 100;
        int client_time_offset = 200;

        std::string application;
        std::string organization;

        std::string light_mask_texture;
        std::string sprite_shadow_texture;
    };

    bool LoadConfig(const char* config_file, Config& config);

    struct UserConfig
    {
        bool fullscreen = false;
    };

    void LoadUserConfig(const char* user_config_file, UserConfig& config);
    void SaveUserConfig(const char* user_config_file, const UserConfig& config);
}
