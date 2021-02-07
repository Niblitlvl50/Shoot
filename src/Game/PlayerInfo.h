
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

    inline const char* PlayerStateToString(PlayerState state)
    {
        switch(state)
        {
            case PlayerState::NOT_SPAWNED:
                return "Not Spawned";
            case PlayerState::ALIVE:
                return "Alive";
            case PlayerState::DEAD:
                return "Dead";
        }

        return "Unknown";
    }

    struct PlayerInfo
    {
        PlayerState player_state;
        uint32_t entity_id;

        math::Quad viewport;
        math::Vector position;
        math::Vector velocity;
        float direction;

        WeaponType weapon_type;
        WeaponState weapon_state;
        int magazine_left;
        int magazine_capacity;
        int ammunition_left;
        int weapon_reload_percentage;

        int score;
        int lifes;
    };

    constexpr int n_players = 4;
    extern PlayerInfo g_players[n_players];
    extern struct NavmeshContext* g_navmesh;

    void InitializeAIKnowledge();
    
    // Might return nullptr if no one is avalible.
    game::PlayerInfo* AllocatePlayerInfo();
    void ReleasePlayerInfo(PlayerInfo* player_info);

    game::PlayerInfo* FindPlayerInfoFromEntityId(uint32_t entity_id);
    const PlayerInfo* GetClosestActivePlayer(const math::Vector& world_position);

    bool IsPlayer(uint32_t entity_id);
}
