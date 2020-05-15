
#pragma once

#include "Math/Vector.h"
#include <vector>

class IEntityManager;

namespace world
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

    std::vector<uint32_t> ReadWorldComponentObjects(const char* file_name, IEntityManager* entity_manager);
}
