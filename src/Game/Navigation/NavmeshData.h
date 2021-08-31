
#pragma once

#include "Math/Vector.h"
#include <vector>

namespace game
{
    struct NavmeshNode
    {
        int data_index;
        int neighbours_index[8];
    };

    struct NavmeshContext
    {
        std::vector<math::Vector> points;
        std::vector<NavmeshNode> nodes;
    };
}
