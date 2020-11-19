
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
        void Idle(const mono::UpdateContext& update_context);

        void ToPrepareAttack();
        void PrepareAttack(const mono::UpdateContext& update_context);

        void ToAttacking();
        void Attacking(const mono::UpdateContext& update_context);

        enum class GoblinStates
        {
            NONE,
            IDLE,
            PREPARE_ATTACK,
            ATTACKING
        };

        uint32_t m_entity_id;
        mono::TransformSystem* m_transform_system;
        std::unique_ptr<IWeaponSystem> m_weapon;
        mono::ISprite* m_sprite;

        using GoblinStateMachine = StateMachine<GoblinStates, const mono::UpdateContext&>;
        GoblinStateMachine m_states;

        uint32_t m_idle_timer;
        uint32_t m_prepare_timer;
        uint32_t m_n_attacks;
        uint32_t m_attack_timer;
    };
}
