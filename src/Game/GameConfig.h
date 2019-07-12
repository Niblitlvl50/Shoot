
#pragma once

namespace game
{
    struct Config
    {
        bool use_port_range = true;
        int client_port = 2100;
        int server_port = 2101;
        int port_range_start = 21000;
        int port_range_end = 22000;
    };

    bool LoadConfig(const char* config_file, Config& config);
}
