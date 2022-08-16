
#include "AIUtils.h"
#include "CollisionConfiguration.h"
#include "Player/PlayerInfo.h"

#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsSpace.h"

bool game::SeesPlayer(mono::PhysicsSystem* physics_system, const math::Vector& position, const game::PlayerInfo* player_info)
{
    const uint32_t query_category = CollisionCategory::PLAYER | CollisionCategory::STATIC;
    mono::PhysicsSpace* space = physics_system->GetSpace();
    const mono::QueryResult result = space->QueryFirst(position, player_info->position, query_category);
    return (result.body != nullptr && result.collision_category & CollisionCategory::PLAYER);
}
