
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Physics/PhysicsFwd.h"
#include "Rendering/RenderFwd.h"
#include "StateMachine.h"

#include "Entity/IEntityLogic.h"
#include "Weapons/IWeaponFactory.h"

#include <memory>

namespace game
{
    class CacodemonController : public IEntityLogic
    {
    public:

        CacodemonController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        ~CacodemonController();

        void Update(const mono::UpdateContext& update_context) override;

        void OnIdle();
        void Idle(const mono::UpdateContext& update_context);

        void OnAttack();
        void Attack(const mono::UpdateContext& update_context);

        void OnDead();
        void Dead(const mono::UpdateContext& update_context);

    private:

        math::Matrix* m_transform;
        mono::IBody* m_entity_body;
        mono::ISprite* m_entity_sprite;
        uint32_t m_idle_animation;
        uint32_t m_attack_animation;
        uint32_t m_death_animation;
        bool m_ready_to_attack;

        IWeaponPtr m_weapon;

        enum class States
        {
            IDLE,
            ATTACK,
            DEAD,
        };

        using CacoStateMachine = StateMachine<States, const mono::UpdateContext&>;
        CacoStateMachine m_states;
    };
}
