
#include "PlayerConfig.h"
#include "Player/Serialize.h"

#include "System/File.h"
#include "nlohmann/json.hpp"

bool game::LoadPlayerConfig(const char* config_file, game::PlayerConfig& config)
{
    file::FilePtr file = file::OpenAsciiFile(config_file);
    if(!file)
        return false;

    const std::vector<byte> file_data = file::FileRead(file);
    config = nlohmann::json::parse(file_data).get<game::PlayerConfig>();

    return true;
}
