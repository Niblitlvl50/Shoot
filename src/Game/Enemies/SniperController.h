
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "Entity/TargetTypes.h"
#include "StateMachine.h"
#include "Math/Vector.h"
#include "Behaviour/HomingBehaviour.h"
#include "Weapons/WeaponFwd.h"

namespace game
{
    class SniperController : public IEntityLogic
    {
    public:

        SniperController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler);
        void Update(const mono::UpdateContext& update_context) override;
        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

    private:

        void ToIdle();
        void Idle(const mono::UpdateContext& update_context);

        void ToReposition();
        void Reposition(const mono::UpdateContext& update_context);

        void ToRetreat();
        void Retreat(const mono::UpdateContext& update_context);

        void ToAim();
        void Aim(const mono::UpdateContext& update_context);

        void ToFire();
        void Fire(const mono::UpdateContext& update_context);

        enum class States { IDLE, REPOSITION, RETREAT, AIM, FIRE };

        uint32_t m_entity_id;
        mono::TransformSystem* m_transform_system;
        class TargetSystem* m_target_system;
        mono::ISprite* m_sprite;
        int m_idle_anim_id;
        int m_run_anim_id;
        IWeaponPtr m_weapon;

        using SniperStateMachine = StateMachine<States, const mono::UpdateContext&>;
        SniperStateMachine m_states;

        float m_aim_timer_s;
        float m_fire_timer_s;
        HomingBehaviour m_homing_movement;
        ITargetPtr m_aquired_target;
    };
}
