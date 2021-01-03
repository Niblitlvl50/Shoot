
#include "SpriteResources.h"
#include "System/File.h"
#include "Util/Hash.h"

#include <unordered_map>
#include <string>

#include "nlohmann/json.hpp"

namespace
{
    std::unordered_map<uint32_t, std::string> g_hash_to_filename;

    bool LoadFileAndHashContent(const char* filename, const char* json_node_name)
    {
        file::FilePtr file = file::OpenAsciiFile(filename);
        if(!file)
            return false;

        const std::vector<byte> file_data = file::FileRead(file);
        const nlohmann::json& json = nlohmann::json::parse(file_data);
        for(const auto& list_entry : json[json_node_name])
        {
            const std::string sprite_string = list_entry;
            g_hash_to_filename[mono::Hash(sprite_string.c_str())] = sprite_string;
        }

        return true;
    }
}

bool game::LoadAllSprites(const char* all_sprites_file)
{
    return LoadFileAndHashContent(all_sprites_file, "all_sprites");
}

bool game::LoadAllTextures(const char* all_textures_file)
{
    return LoadFileAndHashContent(all_textures_file, "all_textures");
}

bool game::LoadAllWorlds(const char* all_worlds_file)
{
    return LoadFileAndHashContent(all_worlds_file, "all_worlds");
}

const char* game::HashToFilename(uint32_t hash)
{
    return g_hash_to_filename[hash].c_str();
}
