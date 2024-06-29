
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "StateMachine.h"
#include "Math/Vector.h"

namespace game
{
    class BirdController : public IEntityLogic
    {
    public:

        BirdController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler);
        void Update(const mono::UpdateContext& update_context) override;

    private:

        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

        void ToIdle();
        void Idle(const mono::UpdateContext& update_context);

        void ToPeck();
        void Peck(const mono::UpdateContext& update_context);

        void ToMoving();
        void Moving(const mono::UpdateContext& update_context);

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

        enum class States
        {
            IDLE,
            PECK,
            MOVING
        };

        using BirdStateMachine = StateMachine<States, const mono::UpdateContext&>;
        BirdStateMachine m_states;
    };
}
