
#include "AIKnowledge.h"
#include "Math/MathFunctions.h"
#include <cstring>
#include <algorithm>

game::PlayerInfo game::g_player_one;
game::PlayerInfo game::g_player_two;

math::Quad game::g_camera_viewport;
game::NavmeshContext* game::g_navmesh = nullptr;

void game::InitializeAIKnowledge()
{
    std::memset(&g_player_one, 0, sizeof(PlayerInfo));
    std::memset(&g_player_two, 0, sizeof(PlayerInfo));
}

bool game::IsPlayer(uint32_t entity_id)
{
    return (g_player_one.entity_id == entity_id || g_player_two.entity_id == entity_id);
}

game::PlayerInfo* game::GetClosestActivePlayer(const math::Vector& world_position)
{
    static PlayerInfo* all_player_infos[] = {
        &g_player_one,
        &g_player_two,
    };

    PlayerInfo* closest_player = nullptr;
    float closest_distance = math::INF;

    for(size_t index = 0; index < std::size(all_player_infos); ++index)
    {
        PlayerInfo* player_info = all_player_infos[index];

        if(player_info->player_state != PlayerState::ALIVE)
            continue;

        const float distance = math::Length(player_info->position - world_position);
        if(distance < closest_distance)
        {
            closest_distance = distance;
            closest_player = player_info;
        }
    }

    return closest_player;
}
