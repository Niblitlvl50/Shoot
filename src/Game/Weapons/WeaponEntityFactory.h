
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "EntitySystem/Entity.h"
#include "Physics/PhysicsFwd.h"

namespace game
{
    class WeaponEntityFactory
    {
    public:

        WeaponEntityFactory(
            mono::IEntityManager* entity_manager,
            mono::TransformSystem* transform_system,
            mono::PhysicsSystem* physics_system,
            class EntityLogicSystem* logic_system,
            class TargetSystem* target_system);

        mono::Entity CreateBulletEntity(
            uint32_t owner_id,
            uint32_t weapon_identifier_hash,
            const struct BulletConfiguration& bullet_config,
            const struct CollisionConfiguration& collision_config,
            const math::Vector& target,
            const math::Vector& velocity,
            float bullet_direction,
            const math::Matrix& transform) const;

        mono::Entity CreateWebberEntity(
            const std::vector<mono::QueryResult>& bodies, const math::Matrix& transform) const;

    private:

        mono::IEntityManager* m_entity_manager;
        mono::TransformSystem* m_transform_system;
        mono::PhysicsSystem* m_physics_system;
        game::EntityLogicSystem* m_logic_system;
        game::TargetSystem* m_target_system;
    };
}
