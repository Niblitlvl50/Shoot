
#include "Resources.h"
#include "System/File.h"
#include "nlohmann/json.hpp"
#include <string>

namespace
{
    std::vector<std::string> g_all_sprite_files;

    const char* g_all_entities_filename = nullptr;
    std::vector<std::string> g_all_entities;

    const char* g_all_textures_filename = nullptr;
    std::vector<std::string> g_all_textures;

    const char* g_all_worlds_filename = nullptr;
    std::vector<std::string> g_all_worlds;

    void WriteListFile(const char* filename, const char* key_name, const std::vector<std::string>& list)
    {
        nlohmann::json json;
        json[key_name] = list;
        const std::string& serialized_json = json.dump(4);

        file::FilePtr file = file::CreateAsciiFile(filename);
        std::fwrite(serialized_json.data(), serialized_json.length(), sizeof(char), file.get());
    }

    std::vector<std::string> ReadListFile(const char* filename, const char* key_name)
    {
        std::vector<std::string> result;

        file::FilePtr file = file::OpenAsciiFile(filename);
        if(file)
        {
            const std::vector<byte> file_data = file::FileRead(file);
            const nlohmann::json& json = nlohmann::json::parse(file_data);
            for(const auto& list_entry : json[key_name])
                result.push_back(list_entry);
        }

        return result;
    }
}

bool editor::LoadAllSprites(const char* all_sprites_file)
{
    file::FilePtr file = file::OpenAsciiFile(all_sprites_file);
    if(!file)
        return false;

    const std::vector<byte> file_data = file::FileRead(file);
    const nlohmann::json& json = nlohmann::json::parse(file_data);
    for(const auto& list_entry : json["all_sprites"])
    {
        const std::string sprite_string = list_entry;
        const size_t slash_pos = sprite_string.find_last_of('/') + 1;
        const std::string sprite_name = sprite_string.substr(slash_pos);

        g_all_sprite_files.push_back(sprite_name);
    }

    return true;
}

const std::vector<std::string>& editor::GetAllSprites()
{
    return g_all_sprite_files;
}

bool editor::LoadAllEntities(const char* all_entities_file)
{
    g_all_entities = ReadListFile(all_entities_file, "all_entities");
    g_all_entities_filename = all_entities_file;

    return true;
}

bool editor::AddNewEntity(const char* new_entity_name)
{
    const auto it = std::find(g_all_entities.begin(), g_all_entities.end(), new_entity_name);
    if(it != g_all_entities.end())
        return false;

    g_all_entities.push_back(new_entity_name);
    std::sort(g_all_entities.begin(), g_all_entities.end());
    WriteListFile(g_all_entities_filename, "all_entities", g_all_entities);

    return true;
}

const std::vector<std::string>& editor::GetAllEntities()
{
    return g_all_entities;
}

bool editor::LoadAllTextures(const char* filename)
{
    g_all_textures = ReadListFile(filename, "all_textures");
    g_all_textures_filename = filename;
    return true;
}

bool editor::AddNewTexture(const char* new_texture_filename)
{
    return false;
}

const std::vector<std::string>& editor::GetAllTextures()
{
    return g_all_textures;
}

bool editor::LoadAllWorlds(const char* filename)
{
    g_all_worlds = ReadListFile(filename, "all_worlds");
    g_all_worlds_filename = filename;
    return false;
}

bool editor::AddNewWorld(const char* world_filename)
{
    const auto it = std::find(g_all_worlds.begin(), g_all_worlds.end(), world_filename);
    if(it != g_all_worlds.end())
        return false;

    g_all_worlds.push_back(world_filename);
    std::sort(g_all_worlds.begin(), g_all_worlds.end());
    WriteListFile(g_all_worlds_filename, "all_worlds", g_all_worlds);

    return true;
}

const std::vector<std::string>& editor::GetAllWorlds()
{
    return g_all_worlds;
}
