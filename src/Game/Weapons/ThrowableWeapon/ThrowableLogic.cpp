
#include "ThrowableLogic.h"
#include "Effects/SmokeEffect.h"

#include "EntitySystem/IEntityManager.h"
#include "Math/EasingFunctions.h"
#include "Math/MathFunctions.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Random.h"


namespace tweak_values
{
    constexpr float meters_per_second = 3.0f;
    constexpr math::EaseFunction ease_function = math::EaseOutCubic;
}

using namespace game;

ThrowableLogic::ThrowableLogic(
    uint32_t entity_id,
    const char* spawned_entity,
    const math::Vector& position,
    const math::Vector& target,
    mono::TransformSystem* transform_system,
    mono::SpriteSystem* sprite_system,
    mono::ParticleSystem* particle_system,
    mono::IEntityManager* entity_manager)
    : m_entity_id(entity_id)
    , m_spawned_entity(spawned_entity)
    , m_target(target)
    , m_transform_system(transform_system)
    , m_sprite_system(sprite_system)
    , m_entity_manager(entity_manager)
    , m_move_timer_s(0.0f)
{
    m_start_position = position;
    m_move_delta = m_start_position - m_target;

    math::Matrix& entity_transform = m_transform_system->GetTransform(m_entity_id);
    math::Position(entity_transform, m_start_position);

    m_smoke_effect = std::make_unique<SmokeEffect>(particle_system, entity_manager);

    const ThrowableStatemachine::StateTable state_table = {
        ThrowableStatemachine::MakeState(ThrowableStates::THROWING, &ThrowableLogic::ToThrowing, &ThrowableLogic::Throwing, this),
        ThrowableStatemachine::MakeState(ThrowableStates::SPAWNING, &ThrowableLogic::ToSpawning, &ThrowableLogic::Spawning, this),
    };
    m_state.SetStateTableAndState(state_table, ThrowableStates::THROWING);
}

void ThrowableLogic::Update(const mono::UpdateContext& update_context)
{
    m_state.UpdateState(update_context);
}

void ThrowableLogic::ToThrowing()
{
    m_move_timer_s = 0.0f;
}

void ThrowableLogic::Throwing(const mono::UpdateContext& update_context)
{
    const float duration_s = math::Length(m_move_delta) / tweak_values::meters_per_second;

    m_move_timer_s += update_context.delta_s;

    math::Vector new_position;
    new_position.x = tweak_values::ease_function(m_move_timer_s, duration_s, m_start_position.x, m_move_delta.x);
    new_position.y = tweak_values::ease_function(m_move_timer_s, duration_s, m_start_position.y, m_move_delta.y);

    math::Matrix& entity_transform = m_transform_system->GetTransform(m_entity_id);
    math::Position(entity_transform, new_position);

    if(m_move_timer_s > duration_s)
        m_state.TransitionTo(ThrowableStates::SPAWNING);
}

void ThrowableLogic::ToSpawning()
{
    const math::Vector position = m_start_position + m_move_delta;
    m_smoke_effect->EmitSmokeAt(position);

    mono::Entity spawned_entity = m_entity_manager->SpawnEntity(m_spawned_entity);
    math::Matrix& spawned_transform = m_transform_system->GetTransform(spawned_entity.id);
    math::Position(spawned_transform, position);

    constexpr float variation_radians = math::ToRadians(10.0f);
    const float rotation = mono::Random(-variation_radians, variation_radians);
    math::RotateZ(spawned_transform, rotation);

    m_transform_system->SetTransformState(spawned_entity.id, mono::TransformState::CLIENT);

    m_sprite_system->SetSpriteEnabled(m_entity_id, false);
    m_spawn_timer_s = 0.0f;
}

void ThrowableLogic::Spawning(const mono::UpdateContext& update_context)
{
    m_spawn_timer_s += update_context.delta_s;

    if(m_spawn_timer_s > 1.0f)
        m_entity_manager->ReleaseEntity(m_entity_id);
}
