
#pragma once

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
        "res/textures/lava2.png"
        };

    constexpr int n_textures = 7;

    inline int FindTextureIndex(const char* texture)
    {
        for(int index = 0; index < n_textures; ++index)
        {
            const bool found = std::strstr(texture, avalible_textures[index]) != nullptr;
            if(found)
                return index;
        }

        return -1;
    }
}
