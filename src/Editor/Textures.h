
#pragma once

#include "Util/Algorithm.h"
#include <cstring>

namespace editor
{
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
        for(unsigned int index = 0; index < mono::arraysize(avalible_textures); ++index)
        {
            const bool found = std::strstr(texture, avalible_textures[index]) != nullptr;
            if(found)
                return index;
        }

        return -1;
    }
}
