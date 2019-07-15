
#include "GameConfig.h"
#include "System/File.h"
#include "nlohmann/json.hpp"

bool game::LoadConfig(const char* config_file, game::Config& config)
{
    file::FilePtr file = file::OpenAsciiFile(config_file);
    if(!file)
        return false;

    std::vector<byte> file_data;
    file::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);

    config.use_port_range   = json.value("use_port_range", config.use_port_range);
    config.client_port      = json.value("client_port", config.client_port);
    config.server_port      = json.value("server_port", config.server_port);
    config.port_range_start = json.value("port_range_start", config.port_range_start);
    config.port_range_end   = json.value("port_range_end", config.port_range_end);

    return true;
}
