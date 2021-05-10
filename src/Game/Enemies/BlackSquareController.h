
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "Physics/IBody.h"
#include "StateMachine.h"

#include <memory>

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
        void ToAwake();
        void ToHunt();

        void SleepState(uint32_t delta);
        void AwakeState(uint32_t delta);
        void HuntState(uint32_t delta);

        const uint32_t m_entity_id;
        mono::EventHandler& m_event_handler;
        const float m_trigger_distance;
        uint32_t m_awake_state_timer;

        using MyStateMachine = StateMachine<States, uint32_t>;
        MyStateMachine m_states;
        std::unique_ptr<class HomingBehaviour> m_homing_behaviour;

        math::Matrix* m_transform;
        mono::ISprite* m_sprite;
        mono::IBody* m_body;
    };
}
