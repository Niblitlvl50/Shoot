
#pragma once

#include "Math/Vector.h"
#include "Math/Quad.h"
#include "Weapons/WeaponTypes.h"
#include "Weapons/WeaponState.h"
#include "Input/InputSystemTypes.h"

#include "System/System.h"

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
        System::ControllerId controller_id;
        uint32_t entity_id;
        uint32_t killer_entity_id;

        math::Quad viewport;
        math::Vector position;
        math::Vector velocity;
        float direction;
        
        math::Vector aim_direction;
        math::Vector aim_target;

        WeaponSetup weapon_type;
        WeaponState weapon_state;
        int magazine_left;

        uint32_t cooldown_id;
        float cooldown_fraction;
        float health_fraction;

        int chips;

        bool laser_sight;
        bool auto_aim;
        bool auto_reload;

        mono::InputContextType last_used_input;
    };

    constexpr int n_players = 3;
    extern PlayerInfo g_players[n_players];


    enum class PackageState
    {
        NOT_SPAWNED,
        SPAWNED,
        SHIELDED,
    };

    inline const char* PackageStateToString(PackageState state)
    {
        switch(state)
        {
            case PackageState::NOT_SPAWNED:
                return "Not Spawned";
            case PackageState::SPAWNED:
                return "Spawned";
            case PackageState::SHIELDED:
                return "Shielded";
        }

        return "Unknown";
    }

    struct PackageInfo
    {
        uint32_t entity_id;
        PackageState state;
        float cooldown_fraction;
    };

    extern PackageInfo g_package_info;

    enum class CoopPowerUpState
    {
        DISABLED,
        ENABLED,
        TRIGGERED
    };

    inline const char* CoopPowerUpStateToString(CoopPowerUpState state)
    {
        switch(state)
        {
            case CoopPowerUpState::DISABLED:
                return "Disabled";
            case CoopPowerUpState::ENABLED:
                return "Enabled";
            case CoopPowerUpState::TRIGGERED:
                return "Triggered";
        }

        return "Unknown";
    }

    struct CoopPowerUp
    {
        CoopPowerUpState state;
        float powerup_value;
    };

    extern CoopPowerUp g_coop_powerup;

    void InitializePlayerInfo();

    constexpr int ANY_PLAYER_INFO = -1;

    // Might return nullptr if no one is avalible.
    game::PlayerInfo* AllocatePlayerInfo(int player_index = ANY_PLAYER_INFO);
    void ReleasePlayerInfo(PlayerInfo* player_info);

    uint32_t FindPlayerIndex(const game::PlayerInfo* player_info);
    game::PlayerInfo* FindPlayerInfoFromEntityId(uint32_t entity_id);
    const PlayerInfo* GetClosestActivePlayer(const math::Vector& world_position);
    const PlayerInfo* GetClosestActivePlayer(const math::Vector& world_position, float max_distance);

    using PlayerArray = std::array<const PlayerInfo*, game::n_players>;

    // You need to check for nullptr for each.
    PlayerArray GetActivePlayers();
    PlayerArray GetSpawnedPlayers();
    PlayerArray GetAlivePlayers();

    bool IsPlayer(uint32_t entity_id);
}
