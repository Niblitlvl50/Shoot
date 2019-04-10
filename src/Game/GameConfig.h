
#pragma once

namespace game
{
    struct Config
    {
        int beacon_port = 1000;
        int client_port = 1001;
        int server_port = 1002;
    };

    bool LoadConfig(const char* config_file, Config& config);
}
