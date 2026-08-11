
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "Entity/TargetTypes.h"
#include "StateMachine.h"
#include "Math/Vector.h"
#include "Behaviour/HomingBehaviour.h"

namespace game
{
    class ShamanController : public IEntityLogic
    {
    public:

        ShamanController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler);
        void Update(const mono::UpdateContext& update_context) override;
        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

    private:

        void ToIdle();
        void Idle(const mono::UpdateContext& update_context);

        void ToFlee();
        void Flee(const mono::UpdateContext& update_context);

        void ToHealCast();
        void HealCast(const mono::UpdateContext& update_context);

        enum class States { IDLE, FLEE, HEAL_CAST };

        uint32_t m_entity_id;
        mono::TransformSystem* m_transform_system;
        class DamageSystem* m_damage_system;
        class TargetSystem* m_target_system;
        mono::ISprite* m_sprite;
        int m_idle_anim_id;
        int m_cast_anim_id;

        using ShamanStateMachine = StateMachine<States, const mono::UpdateContext&>;
        ShamanStateMachine m_states;

        float m_heal_cooldown_s;
        float m_cast_timer_s;
        HomingBehaviour m_homing_movement;
        ITargetPtr m_player_target;
    };
}
