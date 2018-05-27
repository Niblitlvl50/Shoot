
#pragma once

#include "Math/Vector.h"
#include "Math/Quad.h"
#include "Weapons/WeaponTypes.h"

namespace game
{
    struct PlayerInfo
    {
        bool is_active;
        math::Vector position;
        int ammunition_left;
        int ammunition_capacity;
        WeaponType weapon_type;
    };

    extern PlayerInfo g_player_one;
    extern PlayerInfo g_player_two;

    extern math::Quad g_camera_viewport;
    extern struct NavmeshContext* g_navmesh;
}
