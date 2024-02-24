
#pragma once

#include "MonoFwd.h"
#include "IUpdatable.h"
#include "Math/Vector.h"
#include "Physics/PhysicsFwd.h"
#include "Physics/IBody.h"
#include "StateMachine.h"

namespace game
{
    class HookshotLogic : public mono::ICollisionHandler
    {
    public:
        HookshotLogic(
            uint32_t owner_entity_id,
            mono::IEntityManager* entity_system,
            mono::PhysicsSystem* physics_system,
            mono::SpriteSystem* sprite_system,
            mono::TransformSystem* transform_system);
        virtual ~HookshotLogic();

        void TriggerHookshot(const math::Vector& start, float direction);
        void ReleaseHookshot();
        void Update(const mono::UpdateContext& update_context);

    private:

        mono::CollisionResolve OnCollideWith(
            mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t categories) override;
        void OnSeparateFrom(mono::IBody* body) override;

        void OnIdle();
        void Idle(const mono::UpdateContext& update_context);
        void OnAnimating();
        void Animating(const mono::UpdateContext& update_context);
        void OnAttached();
        void Attached(const mono::UpdateContext& update_context);
        void OnDetached();
        void Detached(const mono::UpdateContext& update_context);
        void OnMissed();
        void Missed(const mono::UpdateContext& update_context);

        const uint32_t m_owner_entity_id;
        mono::IEntityManager* m_entity_system;
        mono::PhysicsSystem* m_physics_system;
        mono::SpriteSystem* m_sprite_system;
        mono::TransformSystem* m_transform_system;

        enum class States
        {
            IDLE,
            ANIMATING,
            ATTACHED,
            DETACHED,
            MISSED,
        };

        using HookshotStateMachine = StateMachine<States, const mono::UpdateContext&>;
        HookshotStateMachine m_states;

        mono::IBody* m_attached_to_body;
        math::Vector m_attached_to_local_point;
        mono::IConstraint* m_hookshot_spring;

        uint32_t m_grappler_entity;
    };
}
