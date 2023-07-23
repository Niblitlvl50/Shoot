
#pragma once

#include "Rendering/Sprite/SpriteData.h"

namespace animator
{
    void WriteSpriteFile(const char* sprite_file, const mono::SpriteData* sprite_data);

    bool LoadAllSprites(const char* all_sprites_file);
    const std::vector<std::string>& GetAllSprites();
}
