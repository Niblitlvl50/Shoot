
#pragma once

#include "System/Audio.h"
#include "Physics/IBody.h"

#include "Entity/IEntityLogic.h"
#include "Weapons/WeaponConfiguration.h"
#include "Behaviour/HomingBehaviour.h"
#include "Behaviour/CirculatingBehaviour.h"

#include <vector>

namespace game
{
    class BulletLogic : public game::IEntityLogic, public mono::ICollisionHandler
    {
    public:

        BulletLogic(
            uint32_t entity_id,
            uint32_t owner_entity_id,
            const math::Vector& target,
            const math::Vector& velocity,
            float direction,
            const BulletConfiguration& config,
            const CollisionConfiguration& collision_config,
            mono::PhysicsSystem* physics_system);
        
        void Update(const mono::UpdateContext& update_context) override;
        
        mono::CollisionResolve OnCollideWith(
            mono::IBody* body,
            const math::Vector& collision_point,
            const math::Vector& collision_normal,
            uint32_t categories) override;
        
        void OnSeparateFrom(mono::IBody* body) override;

        const uint32_t m_entity_id;
        const uint32_t m_owner_entity_id;
        const math::Vector m_target;
        BulletImpactCallback m_collision_callback;
        mono::PhysicsSystem* m_physics_system;
        int m_damage;
        uint32_t m_bullet_behaviour;

        float m_life_span;
        audio::ISoundPtr m_sound;

        std::vector<uint32_t> m_jump_ids;
        int m_jumps_left;

        HomingBehaviour m_homing_behaviour;
        CirculatingBehaviour m_circulating_behaviour;
    };
}
