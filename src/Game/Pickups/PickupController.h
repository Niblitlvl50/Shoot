
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"

#include "Entity/IEntityLogic.h"
#include "StateMachine.h"

namespace game
{
    class PickupController : public game::IEntityLogic
    {
    public:

        PickupController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        void Update(const mono::UpdateContext& update_context) override;

        void InitializePosition();
        void MoveToPosition(uint32_t delta_ms);
        void ToWaiting();
        void WaitForPickup(uint32_t delta_ms);

        enum class PickupStates
        {
            MOVE_TO_POSITION,
            WAIT_FOR_PICKUP,
        };

        uint32_t m_entity_id;
        mono::TransformSystem* m_transform_system;

        using PickupStateMachine = StateMachine<PickupStates, uint32_t>;
        PickupStateMachine m_states;

        math::Vector m_target_position;
    };
}
