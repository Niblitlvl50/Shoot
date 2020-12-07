
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "StateMachine.h"

#include <memory>

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

        void Update(const mono::UpdateContext& update_context) override;

    private:

        void ToIdle();
        void ToTracking();
        void ToAttacking();
        void Idle(const mono::UpdateContext& update_context);
        void Tracking(const mono::UpdateContext& update_context);
        void Attacking(const mono::UpdateContext& update_context);

        const uint32_t m_entity_id;

        std::unique_ptr<class IBulletWeapon> m_weapon;
        std::unique_ptr<class TrackingBehaviour> m_tracking_behaviour;

        uint32_t m_idle_timer;
        using InvaderStateMachine = StateMachine<InvaderStates, const mono::UpdateContext&>;
        InvaderStateMachine m_states;

        mono::ISprite* m_sprite;
        math::Matrix* m_transform;
    };
}
