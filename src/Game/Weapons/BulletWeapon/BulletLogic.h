
#pragma once

#include "MonoFwd.h"
#include "System/Audio.h"
#include "Physics/IBody.h"

#include "Entity/IEntityLogic.h"
#include "Weapons/WeaponConfiguration.h"
#include "Behaviour/HomingBehaviour.h"
#include "Behaviour/CirculatingBehaviour.h"
#include "Behaviour/SineWaveBehaviour.h"
#include "Entity/TargetTypes.h"

#include <vector>
#include <string>

namespace game
{
    class BulletLogic : public game::IEntityLogic, public mono::ICollisionHandler
    {
    public:

        BulletLogic(
            uint32_t entity_id,
            uint32_t owner_entity_id,
            uint32_t weapon_identifier_hash,
            const math::Vector& target,
            const math::Vector& velocity,
            float direction,
            const BulletConfiguration& bullet_config,
            const CollisionConfiguration& collision_config,
            mono::TransformSystem* transform_system,
            mono::PhysicsSystem* physics_system,
            class TargetSystem* target_system);
        
        void Update(const mono::UpdateContext& update_context) override;
        
        mono::CollisionResolve OnCollideWith(
            mono::IBody* body,
            const math::Vector& collision_point,
            const math::Vector& collision_normal,
            uint32_t categories) override;
        
        void OnSeparateFrom(mono::IBody* body) override;

        const uint32_t m_entity_id;
        const uint32_t m_owner_entity_id;
        uint32_t m_weapon_identifier_hash;

        const math::Vector m_target;
        BulletImpactCallback m_collision_callback;
        mono::TransformSystem* m_transform_system;
        mono::PhysicsSystem* m_physics_system;
        class TargetSystem* m_target_system;

        bool m_critical_hit;
        int m_damage;
        std::string m_impact_entity;
        uint32_t m_bullet_collision_behaviour;
        uint32_t m_bullet_movement_behaviour;

        float m_life_span;
        bool m_is_player_faction;
        int m_jumps_left;

        audio::ISoundPtr m_sound;
        ITargetPtr m_aquired_target;

        std::vector<uint32_t> m_jump_ids;

        HomingBehaviour m_homing_behaviour;
        CirculatingBehaviour m_circulating_behaviour;
        SineWaveBehaviour m_sinewave_behaviour;
    };
}
