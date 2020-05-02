
#pragma once

#include "Math/Vector.h"
#include "Math/Quad.h"
#include "Weapons/WeaponTypes.h"

#include <cstdint>

namespace game
{
    struct PlayerInfo
    {
        bool is_active;
        uint32_t entity_id;
        math::Vector position;
        WeaponType weapon_type;
        int magazine_left;
        int magazine_capacity;
        int ammunition_left;
        int score;
    };

    extern PlayerInfo g_player_one;
    extern PlayerInfo g_player_two;

    extern math::Quad g_camera_viewport;
    extern struct NavmeshContext* g_navmesh;

    void InitializeAIKnowledge();
}
