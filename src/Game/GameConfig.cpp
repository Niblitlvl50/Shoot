
#include "GameConfig.h"
#include "System/File.h"
#include "nlohmann/json.hpp"

namespace
{
    constexpr const char* beacon_port = "beacon_port";
    constexpr const char* client_port = "client_port";
    constexpr const char* server_port = "server_port";
}

bool game::LoadConfig(const char* config_file, game::Config& config)
{
    file::FilePtr file = file::OpenAsciiFile(config_file);
    if(!file)
        return false;

    std::vector<byte> file_data;
    file::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);

    if(json.count(beacon_port) > 0)
        config.beacon_port = json[beacon_port];

    if(json.count(client_port) > 0)
        config.client_port = json[client_port];

    if(json.count(server_port) > 0)
        config.server_port = json[server_port];

    return true;
}
