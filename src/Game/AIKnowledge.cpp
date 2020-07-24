
#include "AIKnowledge.h"
#include <cstring>

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
