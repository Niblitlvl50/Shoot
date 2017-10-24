
#pragma once

#include "NavmeshData.h"
#include "WorldFile.h"

namespace game
{
    std::vector<math::Vector> GenerateMeshPoints(
        const math::Vector start, float width, float height, float density, const std::vector<world::PolygonData>& static_polygons);
    std::vector<NavmeshNode> GenerateMeshNodes(
        const std::vector<math::Vector>& points, float connection_distance, const std::vector<world::PolygonData>& static_polygons);
}
