
#pragma once

#include "Math/Vector.h"
#include <cstdint>

namespace editor
{
    struct SnapPoint
    {
        uint32_t id = 0;
        float normal = 0;
        math::Vector position;
    };

    struct SnapPair
    {
        SnapPoint snap_from;
        SnapPoint snap_to;
        bool found_snap = false;
    };
}
