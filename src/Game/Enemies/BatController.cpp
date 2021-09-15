
#include "BatController.h"
#include "Player/PlayerInfo.h"

#include "Math/EasingFunctions.h"
#include "TransformSystem/TransformSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "SystemContext.h"
#include "Util/Random.h"

using namespace game;

namespace tweak_values
{
    constexpr float chill_time_min = 0.0f;
    constexpr float chill_time_max = 0.0f;
    constexpr float move_radius = 0.25f;
    constexpr float move_speed = 0.4f;
    constexpr math::EaseFunction ease_function = math::EaseBackOut;
}

BatController::BatController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
{
    m_entity_id = entity_id;
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();

    const BatStateMachine::StateTable state_table = {
        BatStateMachine::MakeState(BatStates::IDLE, &BatController::ToIdle, &BatController::Idle, this),
        BatStateMachine::MakeState(BatStates::MOVING, &BatController::ToMoving, &BatController::Moving, this),
    };

    m_states.SetStateTableAndState(state_table, BatStates::IDLE);

    const math::Matrix& world_transform = m_transform_system->GetWorld(entity_id);
    const math::Vector& world_position = math::GetPosition(world_transform);

    m_start_position = world_position;
    m_current_position = m_start_position;
}

void BatController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

void BatController::ToIdle()
{
    m_chill_time = mono::Random(tweak_values::chill_time_min, tweak_values::chill_time_max);
}

void BatController::Idle(const mono::UpdateContext& update_context)
{
    m_chill_time -= update_context.delta_s;
    if(m_chill_time <= 0.0f)
        m_states.TransitionTo(BatStates::MOVING);
}

void BatController::ToMoving()
{
    const math::Matrix& world_transform = m_transform_system->GetWorld(m_entity_id);
    m_current_position = math::GetPosition(world_transform);

    constexpr float move_radius = tweak_values::move_radius;
    const float x = mono::Random(-move_radius, move_radius);
    const float y = mono::Random(-move_radius, move_radius);

    m_move_delta = (m_start_position + math::Vector(x, y)) - m_current_position;
    m_move_counter = 0.0f;

    mono::ISprite* sprite = m_sprite_system->GetSprite(m_entity_id);

    if(m_move_delta.x < 0.0f)
        sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    else
        sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
}

void BatController::Moving(const mono::UpdateContext& update_context)
{
    const float duration = math::Length(m_move_delta) / tweak_values::move_speed;

    math::Vector new_position;
    new_position.x = tweak_values::ease_function(m_move_counter, duration, m_current_position.x, m_move_delta.x);
    new_position.y = tweak_values::ease_function(m_move_counter, duration, m_current_position.y, m_move_delta.y);

    math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);
    math::Position(transform, new_position);
    m_transform_system->SetTransformState(m_entity_id, mono::TransformState::CLIENT);

    m_move_counter += update_context.delta_s;
    if(m_move_counter >= duration)
        m_states.TransitionTo(BatStates::IDLE);
}
