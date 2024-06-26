
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "Entity/TargetTypes.h"
#include "StateMachine.h"
#include "Weapons/WeaponFwd.h"
#include "Behaviour/TrackingBehaviour.h"
#include "Behaviour/HomingBehaviour.h"

#include <memory>

namespace game
{

    class FlyingMonsterController : public IEntityLogic
    {
    public:

        FlyingMonsterController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler);
        virtual ~FlyingMonsterController();
        void Update(const mono::UpdateContext& update_context) override;
        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

    private:

        enum class States
        {
            IDLE,
            TRACKING,
            REPOSITION,
            ATTACK_ANTICIPATION,
            ATTACKING
        };

        void ToIdle();
        void Idle(const mono::UpdateContext& update_context);

        void ToTracking();
        void Tracking(const mono::UpdateContext& update_context);

        void ToReposition();
        void Reposition(const mono::UpdateContext& update_context);

        void ToAttackAnticipation();
        void AttackAnticipation(const mono::UpdateContext& update_context);

        void ToAttacking();
        void Attacking(const mono::UpdateContext& update_context);

        const uint32_t m_entity_id;

        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;
        class TargetSystem* m_target_system;

        IWeaponPtr m_weapon;
        TrackingBehaviour m_tracking_movement;
        HomingBehaviour m_homing_movement;

        float m_idle_timer_s;
        float m_attack_anticipation_timer_s;
        
        using FlyingMonsterStateMachine = StateMachine<States, const mono::UpdateContext&>;
        FlyingMonsterStateMachine m_states;

        int m_bullets_fired;
        ITargetPtr m_aquired_target;
    };
}
