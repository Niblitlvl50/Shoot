
#pragma once

#include "Math/Vector.h"
#include "Math/Quad.h"

namespace game
{
    struct PlayerInfo
    {
        bool is_active;
        math::Vector position;
        int ammunition;
    };

    extern PlayerInfo player_one;
    extern PlayerInfo player_two;

    extern math::Quad camera_viewport;
    extern struct NavmeshContext* g_navmesh;
}
