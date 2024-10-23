
#pragma once

#include "MonoFwd.h"
#include "Entity/IEntityLogic.h"
#include "Entity/TargetTypes.h"
#include "StateMachine.h"
#include "Weapons/WeaponFwd.h"

namespace game
{
    class BombThrowerController : public IEntityLogic
    {
    public:

        BombThrowerController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler);
        ~BombThrowerController();

        void Update(const mono::UpdateContext& update_context) override;
        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

        void EnterIdle();
        void Idle(const mono::UpdateContext& update_context);

        void EnterPrepareAttack();
        void PrepareAttack(const mono::UpdateContext& update_context);

        void EnterAttack();
        void Attack(const mono::UpdateContext& update_context);

        const uint32_t m_entity_id;
        class TargetSystem* m_target_system;
        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;

        int m_idle_id;
        int m_gesture_id;
        int m_attack_id;

        game::IWeaponPtr m_weapon;
        game::ITargetPtr m_aquired_target;

        float m_idle_timer_s;
        float m_attack_timer_s;
        uint32_t m_n_attacks;

        enum class States
        {
            IDLE,
            PREPARE_ATTACK,
            ATTACK
        };
        using TStateMachine = StateMachine<States, const mono::UpdateContext&>;
        TStateMachine m_states;
    };
}
