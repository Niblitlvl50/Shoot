
#include "PlayerInfo.h"
#include "Math/MathFunctions.h"
#include <cstring>
#include <algorithm>

game::PlayerInfo game::g_players[n_players];
game::NavmeshContext* game::g_navmesh = nullptr;

void game::InitializeAIKnowledge()
{
    std::memset(g_players, 0, sizeof(g_players));
}

game::PlayerInfo* game::AllocatePlayerInfo()
{
    const auto find_func = [](const PlayerInfo& player_info){
        return player_info.player_state == PlayerState::NOT_SPAWNED;
    };

    game::PlayerInfo* found_player_info = std::find_if(std::begin(g_players), std::end(g_players), find_func);
    if(found_player_info != std::end(g_players))
        return found_player_info;

    return nullptr;
}

void game::ReleasePlayerInfo(game::PlayerInfo* player_info_release)
{
    const auto find_func = [player_info_release](const PlayerInfo& player_info){
        return player_info_release == &player_info;
    };

    game::PlayerInfo* it = std::find_if(std::begin(g_players), std::end(g_players), find_func);
    if(it != std::end(g_players))
        std::memset(it, 0, sizeof(game::PlayerInfo));
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

const game::PlayerInfo* game::GetClosestActivePlayer(const math::Vector& world_position)
{
    const PlayerInfo* closest_player = nullptr;
    float closest_distance = math::INF;

    for(size_t index = 0; index < n_players; ++index)
    {
        const PlayerInfo& player_info = g_players[index];
        if(player_info.player_state != PlayerState::ALIVE)
            continue;

        const float distance = std::fabs(math::Length(player_info.position - world_position));
        if(distance < closest_distance)
        {
            closest_distance = distance;
            closest_player = &player_info;
        }
    }

    return closest_player;
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

bool game::IsPlayer(uint32_t entity_id)
{
    return FindPlayerInfoFromEntityId(entity_id) != nullptr;
}
