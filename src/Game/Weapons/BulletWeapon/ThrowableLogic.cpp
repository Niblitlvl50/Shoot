
#include "ThrowableLogic.h"
#include "Weapons/WeaponConfiguration.h"

#include "EntitySystem/IEntityManager.h"
#include "Math/EasingFunctions.h"
#include "Math/MathFunctions.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Random.h"

//#include "Debug/IDebugDrawer.h"

namespace tweak_values
{
    constexpr math::EaseFunction ease_function = math::EaseOutCubic;
}

using namespace game;

ThrowableLogic::ThrowableLogic(
    uint32_t entity_id,
    uint32_t owner_entity_id,
    const math::Vector& position,
    const math::Vector& target,
    const math::Vector& velocity,
    const BulletConfiguration& bullet_config,
    mono::TransformSystem* transform_system,
    mono::IEntityManager* entity_manager)
    : m_entity_id(entity_id)
    , m_owner_entity_id(entity_id)
    , m_spawned_entity(bullet_config.impact_entity_file)
    , m_target(target)
    , m_transform_system(transform_system)
    , m_entity_manager(entity_manager)
    , m_move_timer_s(0.0f)
{
    m_start_position = position;
    m_move_delta = m_target - m_start_position;
    m_meters_per_second = math::Length(velocity);
    m_life_span = bullet_config.life_span + (mono::Random() * bullet_config.fuzzy_life_span);

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
    m_life_span -= update_context.delta_s;
    const bool end_of_life = (m_life_span < 0.0f);

    const float duration_s = math::Length(m_move_delta) / m_meters_per_second;

    m_move_timer_s += update_context.delta_s;

    const math::Vector new_position = {
        tweak_values::ease_function(m_move_timer_s, duration_s, m_start_position.x, m_move_delta.x),
        tweak_values::ease_function(m_move_timer_s, duration_s, m_start_position.y, m_move_delta.y)
    };

    math::Matrix& entity_transform = m_transform_system->GetTransform(m_entity_id);
    math::Position(entity_transform, new_position);

    if(m_move_timer_s > duration_s || end_of_life)
        m_state.TransitionTo(ThrowableStates::SPAWNING);
}

void ThrowableLogic::ToSpawning()
{
    const math::Vector& spawn_position = m_transform_system->GetWorldPosition(m_entity_id);

    constexpr float variation_radians = math::ToRadians(10.0f);
    const float rotation = mono::Random(-variation_radians, variation_radians);
    const math::Matrix spawned_transform = math::CreateMatrixWithPositionRotation(spawn_position, rotation);

    mono::Entity spawned_entity = m_entity_manager->SpawnEntity(m_spawned_entity.c_str());
    m_transform_system->SetTransform(spawned_entity.id, spawned_transform, mono::TransformState::CLIENT);

    // Release this
    m_entity_manager->ReleaseEntity(m_entity_id);
}

void ThrowableLogic::Spawning(const mono::UpdateContext& update_context)
{
}
