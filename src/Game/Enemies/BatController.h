
#pragma once

#include "MonoFwd.h"
#include "Entity/IEntityLogic.h"
#include "Math/Vector.h"
#include "StateMachine.h"

namespace game
{
    class BatController : public IEntityLogic
    {
    public:

        BatController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        void Update(const mono::UpdateContext& update_context) override;

    private:

        void ToIdle();
        void Idle(const mono::UpdateContext& update_context);

        void ToMoving();
        void Moving(const mono::UpdateContext& update_context);

        uint32_t m_entity_id;
        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;
        math::Vector m_start_position;
        math::Vector m_current_position;
        math::Vector m_move_delta;
        float m_move_counter;
        float m_chill_time;

        enum class BatStates
        {
            IDLE,
            MOVING
        };

        using BatStateMachine = StateMachine<BatStates, const mono::UpdateContext&>;
        BatStateMachine m_states;
    };
}
