
#pragma once

#include "Math/Vector.h"
#include <vector>
#include <cstdint>

class IEntityManager;

namespace shared
{
    constexpr size_t PolygonTextureNameMaxLength = 64;

    struct PolygonData
    {
        math::Vector position;
        math::Vector local_offset;
        float rotation = 0.0f;
        char texture[PolygonTextureNameMaxLength] = { 0 };
        std::vector<math::Vector> vertices;
    };

    struct LevelMetadata
    {
        math::Vector camera_position;
        math::Vector camera_size;
    };

    struct LevelData
    {
        LevelMetadata metadata;
        std::vector<uint32_t> loaded_entities;
    };

    LevelData ReadWorldComponentObjects(const char* file_name, IEntityManager* entity_manager);
}
