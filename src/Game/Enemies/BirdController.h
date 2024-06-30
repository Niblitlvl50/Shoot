
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "StateMachine.h"
#include "Math/Vector.h"
#include "Physics/IBody.h"

#include "Behaviour/HomingBehaviour.h"

namespace game
{
    class BirdController : public IEntityLogic, public mono::ICollisionHandler
    {
    public:

        BirdController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler);
        void Update(const mono::UpdateContext& update_context) override;

    private:

        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

        mono::CollisionResolve OnCollideWith(
            mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t categories) override;
        void OnSeparateFrom(mono::IBody* body) override;

        void ToIdle();
        void Idle(const mono::UpdateContext& update_context);

        void ToPeck();
        void Peck(const mono::UpdateContext& update_context);

        void ToMoving();
        void Moving(const mono::UpdateContext& update_context);

        void ToFlying();
        void Flying(const mono::UpdateContext& update_context);
        void ExitFlying();

        uint32_t m_entity_id;
        mono::TransformSystem* m_transform_system;
        mono::ISprite* m_sprite;

        int m_idle_anim_id;
        int m_peck_anim_id;
        int m_walk_anim_id;
        int m_flying_anim_id;

        float m_idle_timer_s;

        math::Vector m_current_position;
        math::Vector m_move_delta;
        float m_move_counter_s;

        float m_total_fly_distance;
        math::Vector m_shadow_offset;
        math::Vector m_current_shadow_offset;
        math::Vector m_current_shadow_offset_velocity;

        game::HomingBehaviour m_homing;

        enum class States
        {
            IDLE,
            PECK,
            MOVING,
            FLYING
        };

        using BirdStateMachine = StateMachine<States, const mono::UpdateContext&>;
        BirdStateMachine m_states;
    };
}
