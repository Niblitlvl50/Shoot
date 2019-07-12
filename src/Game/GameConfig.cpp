
#include "GameConfig.h"
#include "System/File.h"
#include "nlohmann/json.hpp"

namespace
{
    constexpr const char* use_port_range = "use_port_range";
    constexpr const char* client_port = "client_port";
    constexpr const char* server_port = "server_port";
    constexpr const char* port_range_start = "port_range_start";
    constexpr const char* port_range_end = "port_range_end";
}

bool game::LoadConfig(const char* config_file, game::Config& config)
{
    file::FilePtr file = file::OpenAsciiFile(config_file);
    if(!file)
        return false;

    std::vector<byte> file_data;
    file::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);

    if(json.count(use_port_range) > 0)
        config.use_port_range = json[use_port_range];

    if(json.count(client_port) > 0)
        config.client_port = json[client_port];

    if(json.count(server_port) > 0)
        config.server_port = json[server_port];

    if(json.count(port_range_start) > 0)
        config.port_range_start = json[port_range_start];

    if(json.count(port_range_end) > 0)
        config.port_range_end = json[port_range_end];

    return true;
}
