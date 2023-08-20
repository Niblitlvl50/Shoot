
#include "PlayerInfo.h"
#include "Math/MathFunctions.h"
#include "EntitySystem/Entity.h"

#include <cstring>
#include <algorithm>

game::PlayerInfo game::g_players[n_players];
game::PackageInfo game::g_package_info;
game::CoopPowerUp game::g_coop_powerup;

void game::InitializePlayerInfo()
{
    std::memset(g_players, 0, sizeof(g_players));
    std::memset(&g_package_info, 0, sizeof(g_package_info));
    std::memset(&g_coop_powerup, 0, sizeof(g_coop_powerup));

    for(game::PlayerInfo& info : game::g_players)
    {
        info.entity_id = mono::INVALID_ID;
        info.killer_entity_id = mono::INVALID_ID;
        info.last_used_input = mono::InputContextType::None;
    }

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
    {
        std::memset(it, 0, sizeof(game::PlayerInfo));
        it->entity_id = mono::INVALID_ID;
        it->killer_entity_id = mono::INVALID_ID;
        it->last_used_input = mono::InputContextType::None;
    }
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
