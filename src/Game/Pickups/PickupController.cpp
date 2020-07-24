
#include "PickupController.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Random.h"
#include "Math/MathFunctions.h"

#include <functional>

using namespace game;

PickupController::PickupController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();

    using namespace std::placeholders;

    const std::unordered_map<PickupStates, PickupStateMachine::State>& state_table = {
        {
            PickupStates::MOVE_TO_POSITION,
                { std::bind(&PickupController::InitializePosition, this), std::bind(&PickupController::MoveToPosition, this, _1) }
        },
        {
            PickupStates::WAIT_FOR_PICKUP,
                { std::bind(&PickupController::ToWaiting, this), std::bind(&PickupController::WaitForPickup, this, _1) }
        },
    };

    m_states.SetStateTable(state_table);
    m_states.TransitionTo(PickupStates::MOVE_TO_POSITION);
}

void PickupController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context.delta_ms);
}

void PickupController::InitializePosition()
{
    const math::Matrix& world_transform = m_transform_system->GetWorld(m_entity_id);
    const math::Vector& position = math::GetPosition(world_transform);

    const float x_offset = mono::Random(-0.5f, 0.5f);
    const float y_offset = mono::Random(-0.5f, 0.5f);

    m_target_position = position + math::Vector(x_offset, y_offset);
}

void PickupController::MoveToPosition(uint32_t delta_ms)
{
    math::Matrix& world_transform = m_transform_system->GetTransform(m_entity_id);
    const math::Vector& position = math::GetPosition(world_transform);

    //m_position.x = math::EaseOutCubic(m_timer, 0.5f, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);
    //m_position.x = math::EaseOutCubic(m_timer, 0.5f, m_offscreen_position.x, m_screen_position.x - m_offscreen_position.x);


    math::Vector new_position = position; // + move delta
    math::Position(world_transform, new_position);

    const bool is_done = math::IsPrettyMuchEquals(new_position, m_target_position);
    if(is_done)
        m_states.TransitionTo(PickupStates::WAIT_FOR_PICKUP);
}

void PickupController::ToWaiting()
{ }

void PickupController::WaitForPickup(uint32_t delta_ms)
{ }
