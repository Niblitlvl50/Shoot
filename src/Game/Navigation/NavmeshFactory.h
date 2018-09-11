
#pragma once

#include "NavmeshData.h"

namespace world
{
    struct PolygonData;
}

namespace game
{
    struct ExcludeZone
    {
        std::vector<math::Vector> polygon_vertices;
    };

    std::vector<math::Vector> GenerateMeshPoints(
        const math::Vector start, float width, float height, float density, const std::vector<ExcludeZone>& exclude_zones);
    std::vector<NavmeshNode> GenerateMeshNodes(
        const std::vector<math::Vector>& points, float connection_distance, const std::vector<ExcludeZone>& exclude_zones);
}
