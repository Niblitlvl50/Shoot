
#pragma once

#include "MonoFwd.h"
#include "Entity/IEntityLogic.h"
#include "StateMachine.h"

namespace game
{
    class BombThrowerController : public IEntityLogic
    {
    public:

        BombThrowerController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler);

        void Update(const mono::UpdateContext& update_context) override;
        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

        void EnterIdle();
        void Idle(const mono::UpdateContext& update_context);

        void EnterAttack();
        void Attack(const mono::UpdateContext& update_context);

        enum class States
        {
            IDLE,
            ATTACK
        };
        using TStateMachine = StateMachine<States, const mono::UpdateContext&>;
        TStateMachine m_states;
    };
}
