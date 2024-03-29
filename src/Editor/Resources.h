
#pragma once

#include <cstring>
#include <vector>
#include <string>

namespace editor
{
    bool LoadAllSprites(const char* all_sprites_file);
    const std::vector<std::string>& GetAllSprites();

    bool LoadAllMusicTracks(const char* all_music_tracks_file);
    const std::vector<std::string>& GetAllMusicTracks();

    bool LoadAllSounds(const char* all_sounds_file);
    const std::vector<std::string>& GetAllSounds();

    bool LoadAllWeapons(const char* weapon_config_file);
    const std::vector<std::string>& GetAllWeapons();

    bool LoadAllEntities(const char* all_entities_file);
    bool AddNewEntity(const char* new_entity_name);
    const std::vector<std::string>& GetAllEntities();

    bool LoadAllTextures(const char* filename);
    bool AddNewTexture(const char* new_texture_filename);
    const std::vector<std::string>& GetAllTextures();

    bool LoadAllWorlds(const char* filename);
    bool AddNewWorld(const char* world_filename);
    const std::vector<std::string>& GetAllWorlds();
}
