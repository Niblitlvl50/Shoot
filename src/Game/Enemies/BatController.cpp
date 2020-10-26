
#include "BatController.h"
#include "AIKnowledge.h"

#include "Math/EasingFunctions.h"
#include "TransformSystem/TransformSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "SystemContext.h"
#include "Util/Random.h"

using namespace game;

namespace tweek_values
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

    using namespace std::placeholders;
    const std::unordered_map<BatStates, BatStateMachine::State>& state_table = {
        { BatStates::IDLE,      { std::bind(&BatController::ToIdle,     this), std::bind(&BatController::Idle,      this, _1) } },
        { BatStates::MOVING,    { std::bind(&BatController::ToMoving,   this), std::bind(&BatController::Moving,    this, _1) } },
    };

    m_states.SetStateTable(state_table);
    m_states.TransitionTo(BatStates::IDLE);

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
    m_chill_time = mono::Random(tweek_values::chill_time_min, tweek_values::chill_time_max);
}

void BatController::Idle(const mono::UpdateContext& update_context)
{
    m_chill_time -= float(update_context.delta_ms) / 1000.0f;
    if(m_chill_time <= 0.0f)
        m_states.TransitionTo(BatStates::MOVING);
}

void BatController::ToMoving()
{
    const math::Matrix& world_transform = m_transform_system->GetWorld(m_entity_id);
    m_current_position = math::GetPosition(world_transform);

    constexpr float move_radius = tweek_values::move_radius;
    const float x = mono::Random(-move_radius, move_radius);
    const float y = mono::Random(-move_radius, move_radius);

    m_move_delta = (m_start_position + math::Vector(x, y)) - m_current_position;
    m_move_counter = 0.0f;

    const mono::HorizontalDirection new_horizontal_direction =
        m_move_delta.x < 0.0f ? mono::HorizontalDirection::LEFT : mono::HorizontalDirection::RIGHT;

    mono::ISprite* sprite = m_sprite_system->GetSprite(m_entity_id);
    sprite->SetHorizontalDirection(new_horizontal_direction);
}

void BatController::Moving(const mono::UpdateContext& update_context)
{
    const float duration = math::Length(m_move_delta) / tweek_values::move_speed;

    math::Vector new_position;
    new_position.x = tweek_values::ease_function(m_move_counter, duration, m_current_position.x, m_move_delta.x);
    new_position.y = tweek_values::ease_function(m_move_counter, duration, m_current_position.y, m_move_delta.y);

    math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);
    math::Position(transform, new_position);

    m_move_counter += float(update_context.delta_ms) / 1000.0f;

    if(m_move_counter >= duration)
        m_states.TransitionTo(BatStates::IDLE);
}
