
#pragma once

#include "Math/Vector.h"
#include "Math/Quad.h"
#include "Weapons/WeaponTypes.h"
#include "Weapons/WeaponState.h"

#include <cstdint>
#include <array>

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
        int controller_id;
        uint32_t entity_id;
        uint32_t killer_entity_id;

        math::Quad viewport;
        math::Vector position;
        math::Vector velocity;
        float direction;
        float aim_direction;

        WeaponSetup weapon_type;
        WeaponState weapon_state;
        int magazine_left;

        uint32_t cooldown_id;
        float cooldown_fraction;
        bool laser_sight;
    };

    constexpr int n_players = 3;
    extern PlayerInfo g_players[n_players];

    void InitializePlayerInfo();

    constexpr int ANY_PLAYER_INFO = -1;

    // Might return nullptr if no one is avalible.
    game::PlayerInfo* AllocatePlayerInfo(int player_index = ANY_PLAYER_INFO);
    void ReleasePlayerInfo(PlayerInfo* player_info);

    uint32_t FindPlayerIndex(const game::PlayerInfo* player_info);
    game::PlayerInfo* FindPlayerInfoFromEntityId(uint32_t entity_id);
    const PlayerInfo* GetClosestActivePlayer(const math::Vector& world_position);

    using PlayerArray = std::array<const PlayerInfo*, game::n_players>;

    // You need to check for nullptr for each.
    PlayerArray GetActivePlayers();

    bool IsPlayer(uint32_t entity_id);
}
