
#include "PlayerInfo.h"
#include "Math/MathFunctions.h"
#include "EntitySystem/Entity.h"

#include <cstring>
#include <algorithm>

game::PlayerInfo game::g_players[n_players];
game::PackageInfo game::g_package_info;
game::CoopPowerUp game::g_coop_powerup;

namespace
{
    void ClearPlayerInfo(game::PlayerInfo& player_info)
    {
        player_info.player_state = game::PlayerState::NOT_SPAWNED;
        player_info.controller_id = System::ControllerId::Primary;
        player_info.entity_id = mono::INVALID_ID;
        player_info.familiar_entity_id = mono::INVALID_ID;
        player_info.killer_entity_id = mono::INVALID_ID;
        
        player_info.direction = 0.0f;
        player_info.magazine_left = 0;

        player_info.health_fraction = 0.0f;
        player_info.stamina_fraction = 0.0f;
        player_info.player_experience_fraction = 0.0f;
        player_info.weapon_experience_fraction = 0.0f;

        player_info.cooldown_id = 0;
        player_info.cooldown_fraction = 0.0f;

        player_info.powerup_id = 0;
        player_info.powerup_fraction = 0.0f;

        player_info.active_weapon_modifiers.clear();

        player_info.last_used_input = mono::InputContextType::None;

        player_info.persistent_data.chips = 0;
        player_info.persistent_data.rubble = 0;
        player_info.persistent_data.experience = 0;
        player_info.persistent_data.god_mode = false;
        player_info.persistent_data.auto_aim = false;
        player_info.persistent_data.auto_reload = false;
        player_info.persistent_data.laser_sight = false;
        player_info.persistent_data.damage_multiplier = 1.0f;
    }
}

void game::InitializePlayerInfo()
{
    for(game::PlayerInfo& info : game::g_players)
        ClearPlayerInfo(info);

    std::memset(&g_package_info, 0, sizeof(g_package_info));
    std::memset(&g_coop_powerup, 0, sizeof(g_coop_powerup));

    g_package_info.entity_id = mono::INVALID_ID;
}

game::PlayerInfo* game::AllocatePlayerInfo(int player_index)
{
    if(player_index == ANY_PLAYER_INFO)
    {
        const auto find_func = [](const PlayerInfo& player_info){
            return player_info.player_state == PlayerState::NOT_SPAWNED;
        };

        game::PlayerInfo* found_player_info = std::find_if(std::begin(g_players), std::end(g_players), find_func);
        if(found_player_info != std::end(g_players))
            return found_player_info;
    }
    else
    {
        game::PlayerInfo& player_info = g_players[player_index];
        if(player_info.player_state == PlayerState::NOT_SPAWNED)
            return &player_info;
    }

    return nullptr;
}

void game::ReleasePlayerInfo(game::PlayerInfo* player_info_release)
{
    const auto find_func = [player_info_release](const PlayerInfo& player_info){
        return player_info_release == &player_info;
    };

    game::PlayerInfo* it = std::find_if(std::begin(g_players), std::end(g_players), find_func);
    if(it != std::end(g_players))
        ClearPlayerInfo(*it);
}

uint32_t game::FindPlayerIndex(const game::PlayerInfo* player_info)
{
    const auto find_func = [player_info](const PlayerInfo& other_player_info){
        return player_info == &other_player_info;
    };

    game::PlayerInfo* it = std::find_if(std::begin(g_players), std::end(g_players), find_func);
    return std::distance(std::begin(g_players), it);
}

game::PlayerInfo* game::FindPlayerInfoFromEntityId(uint32_t entity_id)
{
    const auto find_func = [entity_id](const PlayerInfo& player_info){
        return entity_id == player_info.entity_id;
    };
    
    game::PlayerInfo* player_info = std::find_if(std::begin(g_players), std::end(g_players), find_func);
    if(player_info != std::end(g_players))
        return player_info;

    return nullptr;
}

game::PlayerArray game::GetActivePlayers()
{
    PlayerArray active_players;

    for(int index = 0; index < game::n_players; ++index)
    {
        game::PlayerInfo& player_info = g_players[index];
        active_players[index] = (player_info.player_state == game::PlayerState::ALIVE) ? &player_info : nullptr;
    }

    return active_players;
}

game::PlayerArray game::GetSpawnedPlayers()
{
    PlayerArray spawned_players;

    for(int index = 0; index < game::n_players; ++index)
    {
        game::PlayerInfo& player_info = g_players[index];
        spawned_players[index] = (player_info.player_state != game::PlayerState::NOT_SPAWNED) ? &player_info : nullptr;
    }

    return spawned_players;
}

game::PlayerArray game::GetAlivePlayers()
{
    PlayerArray players;

    for(int index = 0; index < game::n_players; ++index)
    {
        game::PlayerInfo& player_info = g_players[index];
        players[index] = (player_info.player_state == game::PlayerState::ALIVE) ? &player_info : nullptr;
    }

    return players;
}


bool game::IsPlayer(uint32_t entity_id)
{
    return FindPlayerInfoFromEntityId(entity_id) != nullptr;
}

bool game::IsPlayerOrFamiliar(uint32_t entity_id)
{
    const auto find_func = [entity_id](const PlayerInfo& player_info){
        return player_info.entity_id == entity_id || player_info.familiar_entity_id == entity_id;
    };
    
    game::PlayerInfo* player_info = std::find_if(std::begin(g_players), std::end(g_players), find_func);
    return (player_info != std::end(g_players));
}
