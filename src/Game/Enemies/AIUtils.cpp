
#include "AIUtils.h"
#include "CollisionConfiguration.h"
// #include "Player/PlayerInfo.h"

// #include "EntitySystem/Entity.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsSpace.h"

bool game::SeesPlayer(
    const mono::PhysicsSystem* physics_system, const math::Vector& position, const math::Vector& target_position)
{
    const uint32_t query_category = CollisionCategory::PLAYER | CollisionCategory::STATIC;
    const mono::PhysicsSpace* space = physics_system->GetSpace();
    const mono::QueryResult result = space->QueryFirst(position, target_position, query_category);
    return (result.body != nullptr && result.collision_category & CollisionCategory::PLAYER);
}
