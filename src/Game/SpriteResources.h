
#pragma once

#include <cstdint>

namespace game
{
    bool LoadAllSprites(const char* all_sprites_file);
    const char* SpriteHashToString(uint32_t sprite_hash);
}
