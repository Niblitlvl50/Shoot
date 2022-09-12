
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "StateMachine.h"
#include "Weapons/WeaponFwd.h"
#include "Math/Vector.h"
#include "Behaviour/HomingBehaviour.h"

#include <memory>

namespace game
{
    class ImpController : public IEntityLogic
    {
    public:

        ImpController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        ~ImpController();

        void Update(const mono::UpdateContext& update_context) override;
        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

    private:

        void UpdateWeaponAnimation(const mono::UpdateContext& update_context);

        void ToIdle();
        void Idle(const mono::UpdateContext& update_context);

        void ToReposition();
        void Reposition(const mono::UpdateContext& update_context);

        void ToPrepareAttack();
        void PrepareAttack(const mono::UpdateContext& update_context);

        void ToAttacking();
        void Attacking(const mono::UpdateContext& update_context);

        enum class States
        {
            NONE,
            IDLE,
            REPOSITION,
            PREPARE_ATTACK,
            ATTACKING
        };

        uint32_t m_entity_id;
        uint32_t m_weapon_entity;

        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;
        mono::IEntityManager* m_entity_system;

        IWeaponPtr m_weapon;
        mono::ISprite* m_sprite;
        int m_idle_anim_id;
        int m_run_anim_id;
        int m_attack_anim_id;

        using GoblinStateMachine = StateMachine<States, const mono::UpdateContext&>;
        GoblinStateMachine m_states;

        uint32_t m_idle_timer;
        uint32_t m_n_attacks;
        uint32_t m_attack_timer;

        HomingBehaviour m_homing_behaviour;
        const struct PlayerInfo* m_target_player;
        math::Vector m_attack_position;
    };
}
