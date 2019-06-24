
#pragma once

namespace game
{
    struct Config
    {
        int client_port = 0;
        int server_port = 0;
        int port_range_start = 21000;
        int port_range_end = 22000;
    };

    bool LoadConfig(const char* config_file, Config& config);
}
