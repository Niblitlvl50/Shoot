
#pragma once

#include <cstdint>

namespace game
{
    bool LoadAllSprites(const char* all_sprites_file);
    const char* SpriteHashToString(uint32_t sprite_hash);

    bool LoadAllTextures(const char* all_textures_file);
    const char* TextureHashToString(uint32_t texture_hash);

    bool LoadAllWorlds(const char* all_worlds_file);
    const char* WorldHashToString(uint32_t world_hash);
}
