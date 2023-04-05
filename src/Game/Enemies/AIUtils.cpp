
#include "AIUtils.h"
#include "CollisionConfiguration.h"
#include "Player/PlayerInfo.h"

#include "EntitySystem/Entity.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsSpace.h"

game::AIInfo game::g_ai_info;

void game::InitializeAIInfo()
{
    std::memset(&g_ai_info, 0, sizeof(g_ai_info));

    g_ai_info.behaviour = PrimaryAIBehaviour::TargetPlayer;
}

game::FindTargetResult game::FindAITargetFromPosition(const math::Vector& world_position)
{
    FindTargetResult result;
    result.entity_id = mono::INVALID_ID;

    switch(g_ai_info.behaviour)
    {
    case PrimaryAIBehaviour::TargetPlayer:
    {
        const game::PlayerInfo* player_info = game::GetClosestActivePlayer(world_position);
        if(player_info)
        {
            result.entity_id = player_info->entity_id;
            result.world_position = player_info->position;
        }

        break;
    }
    case PrimaryAIBehaviour::TargetPackage:
    {
        result.entity_id = g_package_info.entity_id;
        result.world_position = g_package_info.position;
        break;
    }
    }

    return result;
}

bool game::SeesPlayer(
    const mono::PhysicsSystem* physics_system, const math::Vector& position, const game::PlayerInfo* player_info)
{
    const uint32_t query_category = CollisionCategory::PLAYER | CollisionCategory::STATIC;
    const mono::PhysicsSpace* space = physics_system->GetSpace();
    const mono::QueryResult result = space->QueryFirst(position, player_info->position, query_category);
    return (result.body != nullptr && result.collision_category & CollisionCategory::PLAYER);
}
