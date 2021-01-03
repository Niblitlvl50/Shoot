
#include "SpriteResources.h"
#include "System/File.h"
#include "Util/Hash.h"

#include <unordered_map>
#include <string>

#include "nlohmann/json.hpp"

namespace
{
    std::unordered_map<uint32_t, std::string> g_sprite_hash_to_filename;
    std::unordered_map<uint32_t, std::string> g_texture_hash_to_filename;
    std::unordered_map<uint32_t, std::string> g_world_hash_to_filename;

    std::unordered_map<uint32_t, std::string> LoadFileAndHash(const char* filename, const char* json_node_name)
    {
        std::unordered_map<uint32_t, std::string> hash_to_filename;

        file::FilePtr file = file::OpenAsciiFile(filename);
        if(!file)
            return hash_to_filename;

        const std::vector<byte> file_data = file::FileRead(file);
        const nlohmann::json& json = nlohmann::json::parse(file_data);
        for(const auto& list_entry : json[json_node_name])
        {
            const std::string sprite_string = list_entry;
            hash_to_filename[mono::Hash(sprite_string.c_str())] = sprite_string;
        }

        return hash_to_filename;
    }
}

bool game::LoadAllSprites(const char* all_sprites_file)
{
    g_sprite_hash_to_filename = LoadFileAndHash(all_sprites_file, "all_sprites");
    return true;
}

const char* game::SpriteHashToString(uint32_t sprite_hash)
{
    return g_sprite_hash_to_filename[sprite_hash].c_str();
}

bool game::LoadAllTextures(const char* all_textures_file)
{
    g_texture_hash_to_filename = LoadFileAndHash(all_textures_file, "all_textures");
    return true;
}

const char* game::TextureHashToString(uint32_t texture_hash)
{
    return g_texture_hash_to_filename[texture_hash].c_str();
}

bool game::LoadAllWorlds(const char* all_worlds_file)
{
    g_world_hash_to_filename = LoadFileAndHash(all_worlds_file, "all_worlds");
    return true;
}

const char* game::WorldHashToString(uint32_t world_hash)
{
    return g_world_hash_to_filename[world_hash].c_str();
}
