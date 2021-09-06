
#pragma once

#include "NavmeshData.h"
#include <functional>

namespace game
{
    using NavmeshConnectionFilter = std::function<bool (const math::Vector& first, const math::Vector& second)>;

    std::vector<math::Vector> GenerateMeshPoints(const math::Vector start, const math::Vector& end, float density);
    std::vector<NavmeshNode> GenerateMeshNodes(const std::vector<math::Vector>& points, float connection_distance, NavmeshConnectionFilter filter_function);
}
