
#pragma once

#include "Math/Vector.h"

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
}
