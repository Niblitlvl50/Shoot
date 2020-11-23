
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "StateMachine.h"
#include "Math/Vector.h"

namespace game
{
    class BlobController : public IEntityLogic
    {
    public:

        BlobController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        void Update(const mono::UpdateContext& update_context) override;

    private:

        void ToIdle();
        void Idle(const mono::UpdateContext& update_context);

        void ToMoving();
        void Moving(const mono::UpdateContext& update_context);

        uint32_t m_entity_id;
        mono::TransformSystem* m_transform_system;
        mono::ISprite* m_sprite;

        int m_idle_anim_id;
        int m_jump_anim_id;
        uint32_t m_jump_anim_length;

        uint32_t m_idle_timer;

        math::Vector m_current_position;
        math::Vector m_move_delta;
        float m_move_counter;

        enum class BlobStates
        {
            IDLE,
            MOVING
        };

        using BlobStateMachine = StateMachine<BlobStates, const mono::UpdateContext&>;
        BlobStateMachine m_states;
    };
}
