
#pragma once

#include "NavmeshData.h"

namespace game
{
    struct NavmeshContext;

    std::vector<int> AStar(const NavmeshContext& context, int start, int end);
    std::vector<math::Vector> PathToPoints(const NavmeshContext& context, const std::vector<int>& path);

    int FindClosestIndex(const NavmeshContext& context, const math::Vector& point);
}
