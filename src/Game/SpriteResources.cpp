
#include "SpriteResources.h"
#include "System/File.h"
#include "Util/Hash.h"

#include <unordered_map>
#include <string>

#include "nlohmann/json.hpp"

namespace
{
    std::unordered_map<uint32_t, std::string> g_hash_to_filename;
}

bool game::LoadAllSprites(const char* all_sprites_file)
{
    file::FilePtr file = file::OpenAsciiFile(all_sprites_file);
    if(!file)
        return false;

    std::vector<byte> file_data;
    file::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);
    for(const auto& list_entry : json["all_sprites"])
    {
        const std::string sprite_string = list_entry;
        g_hash_to_filename[mono::Hash(sprite_string.c_str())] = sprite_string;
    }

    return true;
}

const char* game::SpriteHashToString(uint32_t sprite_hash)
{
    return g_hash_to_filename[sprite_hash].c_str();
}
