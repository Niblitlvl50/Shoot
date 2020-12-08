
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

    private:

        const uint32_t m_entity_id;
        math::Matrix* m_transform;
        mono::IBody* m_entity_body;
        mono::ISprite* m_entity_sprite;
        uint32_t m_idle_animation;
        uint32_t m_attack_animation;

        IWeaponPtr m_weapon;

        enum class CacoStates
        {
            IDLE,
            ATTACK
        };

        using CacoStateMachine = StateMachine<CacoStates, const mono::UpdateContext&>;
        CacoStateMachine m_states;
    };
}
