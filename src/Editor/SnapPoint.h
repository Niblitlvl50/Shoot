
#pragma once

#include "Prefabs.h"
#include "Math/Vector.h"

namespace editor
{
    struct SnapPair
    {
        SnapPoint snap_from;
        SnapPoint snap_to;
        bool found_snap = false;
    };
}
