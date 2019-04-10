
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "StateMachine.h"

namespace game
{
    enum class InvaderStates
    {
        IDLE,
        TRACKING,
        ATTACKING
    };

    class InvaderController : public IEntityLogic
    {
    public:

        InvaderController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        virtual ~InvaderController();

        void Update(uint32_t delta_ms) override;

    private:

        void ToIdle();
        void ToTracking();
        void ToAttacking();
        void Idle(uint32_t delta_ms);
        void Tracking(uint32_t delta_ms);
        void Attacking(uint32_t delta_ms);

        const uint32_t m_entity_id;

        std::unique_ptr<class IWeaponSystem> m_weapon;
        std::unique_ptr<class TrackingBehaviour> m_tracking_behaviour;

        uint32_t m_idle_timer;
        using InvaderStateMachine = StateMachine<InvaderStates, uint32_t>;
        InvaderStateMachine m_states;

        mono::ISprite* m_sprite;
        math::Matrix* m_transform;
    };
}
