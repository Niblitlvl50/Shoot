
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "StateMachine.h"
#include "Weapons/WeaponFwd.h"
#include "Math/Vector.h"

#include <memory>

namespace game
{
    class TurretSpawnerController : public IEntityLogic
    {
    public:

        TurretSpawnerController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler);
        void Update(const mono::UpdateContext& update_context) override;

    private:

        void ToIdle();
        void Idle(const mono::UpdateContext& update_context);

        void ToPrepareAttack();
        void PrepareAttack(const mono::UpdateContext& update_context);

        void ToAttacking();
        void Attacking(const mono::UpdateContext& update_context);

        enum class States
        {
            NONE,
            IDLE,
            PREPARE_ATTACK,
            ATTACKING
        };

        uint32_t m_entity_id;
        mono::TransformSystem* m_transform_system;
        IWeaponPtr m_weapon;
        mono::ISprite* m_sprite;

        using TurretStateMachine = StateMachine<States, const mono::UpdateContext&>;
        TurretStateMachine m_states;

        float m_idle_timer_s;
        float m_prepare_timer_s;

        math::Vector m_attack_position;
    };
}
