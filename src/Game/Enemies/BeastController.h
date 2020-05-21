
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "StateMachine.h"

namespace game
{
    class BeastController : public IEntityLogic
    {
    public:

        BeastController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        void Update(const mono::UpdateContext& update_context) override;

    private:

        void ToIdle();
        void ToTracking();
        void ToAttacking();
        void Idle(uint32_t delta);
        void Tracking(uint32_t delta);
        void Attacking(uint32_t delta);

        enum class BeastStates
        {
            IDLE,
            TRACKING,
            ATTACKING
        };

        using BeastStateMachine = StateMachine<BeastStates, uint32_t>;
        BeastStateMachine m_states;
        uint32_t m_timer;
        mono::ISprite* m_sprite;
    };
}
