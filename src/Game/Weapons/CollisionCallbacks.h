
#pragma once

#include "MonoFwd.h"
#include "Physics/PhysicsFwd.h"
#include "WeaponConfiguration.h"

#include <cstdint>

namespace game
{
    class DamageSystem;

    void InitWeaponCallbacks(mono::SystemContext* system_context);
    void CleanupWeaponCallbacks();

    void SpawnEntityWithAnimation(
        const char* entity_file,
        int animation_id,
        uint32_t position_at_transform_id,
        mono::IEntityManager* entity_manager,
        mono::TransformSystem* transform_system,
        mono::SpriteSystem* sprite_system);

    void StandardCollision(
        uint32_t entity_id,
        uint32_t owner_entity_id,
        game::BulletCollisionFlag flags,
        const CollisionDetails& details,
        mono::IEntityManager* entity_manager,
        game::DamageSystem* damage_system,
        mono::PhysicsSystem* physics_system,
        mono::SpriteSystem* sprite_system,
        mono::TransformSystem* transform_system);

    void RocketCollision(
        uint32_t entity_id,
        uint32_t owner_entity_id,
        game::BulletCollisionFlag flags,
        const CollisionDetails& details,
        mono::IEntityManager* entity_manager,
        game::DamageSystem* damage_system,
        mono::PhysicsSystem* physics_system,
        mono::SpriteSystem* sprite_system,
        mono::TransformSystem* transform_system);

    void CacoPlasmaCollision(
        uint32_t entity_id,
        uint32_t owner_entity_id,
        game::BulletCollisionFlag flags,
        const CollisionDetails& details,
        mono::IEntityManager* entity_manager,
        game::DamageSystem* damage_system,
        mono::PhysicsSystem* physics_system,
        mono::SpriteSystem* sprite_system,
        mono::TransformSystem* transform_system);
}
