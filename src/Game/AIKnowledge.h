
#pragma once

#include "Math/Vector.h"
#include "Math/Quad.h"
#include "Weapons/WeaponTypes.h"
#include "Weapons/WeaponState.h"

#include <cstdint>

namespace game
{
    enum class PlayerState
    {
        NOT_SPAWNED,
        ALIVE,
        DEAD,
    };

    struct PlayerInfo
    {
        PlayerState player_state;
        uint32_t entity_id;
        math::Vector position;
        math::Vector velocity;
        float direction;
        WeaponType weapon_type;
        WeaponState weapon_state;
        int magazine_left;
        int magazine_capacity;
        int ammunition_left;
        uint32_t weapon_reload_time_ms;
        int score;
    };

    extern PlayerInfo g_player_one;
    extern PlayerInfo g_player_two;

    extern math::Quad g_camera_viewport;
    extern struct NavmeshContext* g_navmesh;

    void InitializeAIKnowledge();
    bool IsPlayer(uint32_t entity_id);
    PlayerInfo* GetClosestActivePlayer(const math::Vector& world_position);
}
