
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "Entity/TargetTypes.h"
#include "StateMachine.h"
#include "Weapons/WeaponFwd.h"
#include "Math/Vector.h"
#include "Behaviour/HomingBehaviour.h"
#include "Behaviour/TrackingBehaviour.h"

#include <memory>

namespace game
{
    class ImpController : public IEntityLogic
    {
    public:

        ImpController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler);
        ~ImpController();

        void Update(const mono::UpdateContext& update_context) override;
        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

    private:

        void ToIdle();
        void Idle(const mono::UpdateContext& update_context);

        void ToTracking();
        void Tracking(const mono::UpdateContext& update_context);

        void ToReposition();
        void Reposition(const mono::UpdateContext& update_context);

        void ToPrepareAttack();
        void PrepareAttack(const mono::UpdateContext& update_context);

        void ToAttacking();
        void Attacking(const mono::UpdateContext& update_context);

        enum class States
        {
            IDLE,
            TRACKING,
            REPOSITION,
            PREPARE_ATTACK,
            ATTACKING
        };

        uint32_t m_entity_id;

        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;
        class NavigationSystem* m_navigation_system;
        class TargetSystem* m_target_system;

        IWeaponPtr m_weapon;
        mono::ISprite* m_sprite;
        int m_idle_anim_id;
        int m_run_anim_id;
        int m_attack_anim_id;

        using GoblinStateMachine = StateMachine<States, const mono::UpdateContext&>;
        GoblinStateMachine m_states;

        float m_idle_timer_s;
        float m_attack_timer_s;
        uint32_t m_n_attacks;

        HomingBehaviour m_homing_movement;
        TrackingBehaviour m_tracking_movement;
        ITargetPtr m_aquired_target;
    };
}
