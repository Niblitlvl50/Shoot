
#pragma once

#include "NavmeshData.h"

namespace game
{
    struct NavmeshContext;

    enum class AStarResult
    {
        FAILED,
        NO_PATH,
        SUCCESS
    };

    struct NavigationResult
    {
        AStarResult result;
        int nodes_evaluated;
        std::vector<int> path_indices;
    };

    NavigationResult AStar(const NavmeshContext& context, int start_index, int end_index);
    NavigationResult AStar(const NavmeshContext& context, const math::Vector& start, const math::Vector& end);
    std::vector<math::Vector> PathToPoints(const NavmeshContext& context, const std::vector<int>& path);

    int FindClosestIndex(const NavmeshContext& context, const math::Vector& point);
}
