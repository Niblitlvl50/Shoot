
#pragma once

#include "Math/Vector.h"

#include <vector>
#include <string>
#include <cstdint>

namespace game
{
    struct SpriteAtlasMeta
    {
        std::string app;
        std::string version;

        std::string image_filename;
        int image_width;
        int image_height;
    };

    struct SpriteAtlasFrame
    {
        std::string filename;
        uint32_t filename_hash;
        math::Vector xy;
        math::Vector wh;
    };

    struct SpriteAtlasData
    {
        SpriteAtlasMeta meta;
        std::vector<SpriteAtlasFrame> frames;
    };

    SpriteAtlasData ReadSpriteAtlas(const char* filename);
}
