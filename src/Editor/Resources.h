
#pragma once

#include <cstring>
#include <vector>
#include <string>

namespace editor
{
    constexpr const char* body_types[] = {
        "Dynamic",
        "Kinetic",
        "Static"
    };

    static const char* avalible_textures[] = {
        "res/textures/placeholder.png",
        "res/textures/stone_tileable.png",
        "res/textures/gray_stone.png",
        "res/textures/dark_stone.png",
        "res/textures/brown_stone.png",
        "res/textures/lava1.png",
        "res/textures/lava2.png",
        "res/textures/white_box_placeholder.png"
    };

    inline int FindTextureIndex(const char* texture)
    {
        for(uint32_t index = 0; index < std::size(avalible_textures); ++index)
        {
            const bool found = std::strstr(texture, avalible_textures[index]) != nullptr;
            if(found)
                return index;
        }

        return -1;
    }

    bool LoadAllSprites(const char* all_sprites_file);
    const std::vector<std::string>& GetAllSprites();

    bool LoadAllEntities(const char* all_entities_file);
    bool AddNewEntity(const char* new_entity_name);
    const std::vector<std::string>& GetAllEntities();

    bool LoadAllPaths(const char* all_paths_file);
    bool AddNewPath(const char* new_path_name);
    const std::vector<std::string>& GetAllPaths();

    bool LoadAllTextures(const char* filename);
    bool AddNewTexture(const char* new_texture_filename);
    const std::vector<std::string>& GetAllTextures();

    bool LoadAllWorlds(const char* filename);
    const std::vector<std::string>& GetAllWorlds();
}
