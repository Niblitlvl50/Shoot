
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "Physics/IBody.h"
#include "StateMachine.h"

namespace game
{
    class BlackSquareController : public IEntityLogic, public mono::ICollisionHandler
    {
    public:

        BlackSquareController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        virtual ~BlackSquareController();

        void Update(const mono::UpdateContext& update_context) override;
        mono::CollisionResolve OnCollideWith(
            mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t category) override;
        void OnSeparateFrom(mono::IBody* body) override;

    private:

        enum class States
        {
            SLEEPING,
            AWAKE,
            HUNT
        };

        void ToSleep();
        void SleepState(const mono::UpdateContext& update_context);

        void ToAwake();
        void AwakeState(const mono::UpdateContext& update_context);

        void ToHunt();
        void HuntState(const mono::UpdateContext& update_context);

        const uint32_t m_entity_id;
        mono::EventHandler& m_event_handler;
        uint32_t m_awake_state_timer;

        using MyStateMachine = StateMachine<States, const mono::UpdateContext&>;
        MyStateMachine m_states;

        math::Matrix* m_transform;
        mono::ISprite* m_sprite;
        mono::IBody* m_body;

        const math::Vector* m_target_position;

        mono::IEntityManager* m_entity_manager;
        mono::PhysicsSystem* m_physics_system;
        class DamageSystem* m_damage_system;
    };
}
