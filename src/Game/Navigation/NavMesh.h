
#pragma once

#include "Math/Vector.h"
#include "WorldFile.h"
#include <vector>
#include <array>

namespace game
{
    struct NavmeshNode
    {
        int data_index = -1;
        std::array<int, 8> neighbours_index = { {-1} };
    };

    struct NavmeshContext
    {
        std::vector<math::Vector> points;
        std::vector<NavmeshNode> nodes;
    };

    std::vector<math::Vector> GenerateMeshPoints(
        const math::Vector start, float width, float height, float density, const std::vector<world::PolygonData>& static_polygons);
    std::vector<NavmeshNode> GenerateMeshNodes(
        const std::vector<math::Vector>& points, float connection_distance, const std::vector<world::PolygonData>& static_polygons);

    std::vector<int> AStar(const NavmeshContext& context, int start, int end);


    int FindClosestIndex(const NavmeshContext& context, const math::Vector& point);
}
