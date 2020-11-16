
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "StateMachine.h"
#include "Weapons/IWeaponSystem.h"

#include <memory>

namespace game
{
    class GoblinFireController : public IEntityLogic
    {
    public:

        GoblinFireController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        void Update(const mono::UpdateContext& update_context) override;

    private:

        void ToIdle();
        void ToHurt();
        void ToAttacking();
        void Idle(const mono::UpdateContext& update_context);
        void Hurt(const mono::UpdateContext& update_context);
        void Attacking(const mono::UpdateContext& update_context);

        enum class GoblinStates
        {
            IDLE,
            ATTACKING,
            HURT
        };

        uint32_t m_entity_id;
        mono::TransformSystem* m_transform_system;
        std::unique_ptr<IWeaponSystem> m_weapon;
        mono::ISprite* m_sprite;

        using GoblinStateMachine = StateMachine<GoblinStates, const mono::UpdateContext&>;
        GoblinStateMachine m_states;

        uint32_t m_idle_timer;
 
        uint32_t m_n_attacks;
        uint32_t m_attack_timer;
    };
}
