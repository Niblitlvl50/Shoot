
#pragma once

#include <cstdint>

namespace game
{
    bool LoadAllSprites(const char* all_sprites_file);
    bool LoadAllTextures(const char* all_textures_file);
    bool LoadAllWorlds(const char* all_worlds_file);

    const char* HashToFilename(uint32_t hash);
}
