
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
