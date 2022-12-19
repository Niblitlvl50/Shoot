
#include "BlobController.h"

#include "SystemContext.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Random.h"
#include "Math/EasingFunctions.h"


namespace tweak_values
{
    constexpr float idle_threshold_s = 0.5f;
    constexpr int percentage_to_move = 50;
    constexpr float move_radius = 0.25f;
    constexpr math::EaseFunction ease_function = math::EaseInOutCubic;
}

using namespace game;

BlobController::BlobController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    m_idle_anim_id = m_sprite->GetAnimationIdFromName("idle");
    m_jump_anim_id = m_sprite->GetAnimationIdFromName("jump");

    m_jump_anim_length = m_sprite->GetAnimationLengthSeconds(m_jump_anim_id);

    const BlobStateMachine::StateTable state_table = {
        BlobStateMachine::MakeState(States::IDLE, &BlobController::ToIdle, &BlobController::Idle, this),
        BlobStateMachine::MakeState(States::MOVING, &BlobController::ToMoving, &BlobController::Moving, this),
    };

    m_states.SetStateTableAndState(state_table, States::IDLE);
}

void BlobController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

void BlobController::ToIdle()
{
    m_idle_timer_s = 0.0f;
    m_sprite->SetAnimation(m_idle_anim_id);
}

void BlobController::Idle(const mono::UpdateContext& update_context)
{
    m_idle_timer_s += update_context.delta_s;

    if(m_idle_timer_s > tweak_values::idle_threshold_s)
    {
        const bool move = mono::Chance(tweak_values::percentage_to_move);
        if(move)
            m_states.TransitionTo(States::MOVING);

        m_idle_timer_s = 0.0f;
    }
}

void BlobController::ToMoving()
{
    const math::Matrix& world_transform = m_transform_system->GetWorld(m_entity_id);
    m_current_position = math::GetPosition(world_transform);

    constexpr float move_radius = tweak_values::move_radius;
    const float x = mono::Random(-move_radius, move_radius);
    const float y = mono::Random(-move_radius, move_radius);

    m_move_delta = math::Vector(x, y);
    m_move_counter_s = 0.0f;

    if(m_move_delta.x < 0.0f)
        m_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    else
        m_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

    m_sprite->SetAnimation(m_jump_anim_id);
}

void BlobController::Moving(const mono::UpdateContext& update_context)
{
    const float duration = m_jump_anim_length;

    math::Vector new_position;
    new_position.x = tweak_values::ease_function(m_move_counter_s, duration, m_current_position.x, m_move_delta.x);
    new_position.y = tweak_values::ease_function(m_move_counter_s, duration, m_current_position.y, m_move_delta.y);

    math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);
    math::Position(transform, new_position);
    m_transform_system->SetTransformState(m_entity_id, mono::TransformState::CLIENT);

    m_move_counter_s += update_context.delta_s;

    if(m_move_counter_s > duration)
        m_states.TransitionTo(States::IDLE);
}
