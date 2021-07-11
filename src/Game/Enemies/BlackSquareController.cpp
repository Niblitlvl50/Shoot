
#include "BlackSquareController.h"

#include "Player/PlayerInfo.h"
#include "CollisionConfiguration.h"
#include "DamageSystem.h"
#include "Shockwave.h"

#include "Physics/IBody.h"
#include "Physics/PhysicsSystem.h"

#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "SystemContext.h"

#include "EntitySystem/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"


namespace tweak_values
{
    constexpr float trigger_distance = 5.0f;
    constexpr uint32_t time_before_hunt_ms = 300;
    constexpr uint32_t collision_damage = 45;
    constexpr float shockwave_magnitude = 5.0f;

    constexpr float degrees_per_second = 120.0f;
    constexpr float velocity = 0.1f;
}

using namespace game;

BlackSquareController::BlackSquareController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
    , m_event_handler(event_handler)
    , m_awake_state_timer(0)
    , m_current_heading(0.0f)
{
    mono::TransformSystem* transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_transform = &transform_system->GetTransform(entity_id);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_body = m_physics_system->GetBody(entity_id);
    m_body->AddCollisionHandler(this);

    m_entity_manager = system_context->GetSystem<mono::IEntityManager>();
    m_damage_system = system_context->GetSystem<game::DamageSystem>();

    using namespace std::placeholders;

    const MyStateMachine::StateTable& state_table = {
        MyStateMachine::MakeState(States::SLEEPING, &BlackSquareController::ToSleep, &BlackSquareController::SleepState, this),
        MyStateMachine::MakeState(States::AWAKE,    &BlackSquareController::ToAwake, &BlackSquareController::AwakeState, this),
        MyStateMachine::MakeState(States::HUNT,     &BlackSquareController::ToHunt,  &BlackSquareController::HuntState, this),
    };
    m_states.SetStateTableAndState(state_table, States::SLEEPING);
}

BlackSquareController::~BlackSquareController()
{ }

void BlackSquareController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

mono::CollisionResolve BlackSquareController::OnCollideWith(
    mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t category)
{
    if(m_states.ActiveState() == States::SLEEPING)
        m_states.TransitionTo(States::AWAKE);

    if(category == shared::CollisionCategory::PLAYER)
    {
        const math::Vector& entity_position = math::GetPosition(*m_transform);
        game::ShockwaveAt(m_physics_system, entity_position, tweak_values::shockwave_magnitude);

        const uint32_t other_entity_id = mono::PhysicsSystem::GetIdFromBody(body);
        //m_damage_system->ApplyDamage(other_entity_id, tweak_values::collision_damage, m_entity_id);
        m_entity_manager->ReleaseEntity(m_entity_id);
    }

    return mono::CollisionResolve::NORMAL;
}

void BlackSquareController::OnSeparateFrom(mono::IBody* body)
{ }

void BlackSquareController::ToSleep()
{
    m_body->ResetForces();
}

void BlackSquareController::SleepState(const mono::UpdateContext& update_context)
{
    const math::Vector& entity_position = math::GetPosition(*m_transform);
    const game::PlayerInfo* player_info = GetClosestActivePlayer(entity_position);
    if(!player_info)
        return;

    const float distance = math::Length(player_info->position - entity_position);
    if(distance < tweak_values::trigger_distance)
        m_states.TransitionTo(States::AWAKE);
}

void BlackSquareController::ToAwake()
{
    m_sprite->SetShade(mono::Color::WHITE);
    m_awake_state_timer = 0;
}

void BlackSquareController::AwakeState(const mono::UpdateContext& update_context)
{
    m_awake_state_timer += update_context.delta_ms;
    if(m_awake_state_timer > tweak_values::time_before_hunt_ms)
        m_states.TransitionTo(States::HUNT);
}

void BlackSquareController::ToHunt()
{
    const math::Vector& entity_position = math::GetPosition(*m_transform);
    const game::PlayerInfo* player_info = game::GetClosestActivePlayer(entity_position);
    m_target_position = (player_info != nullptr) ? &player_info->position : nullptr;
}

void BlackSquareController::HuntState(const mono::UpdateContext& update_context)
{
    if(!m_target_position)
    {
        m_states.TransitionTo(States::SLEEPING);
        return;
    }

    const math::Vector& entity_position = math::GetPosition(*m_transform);
    const math::Vector& target_position = *m_target_position;

    const math::Vector& delta = math::Normalized(target_position - entity_position);

    if(delta.x < 0.0f)
        m_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    else
        m_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

    const math::Vector& vector_angle = math::VectorFromAngle(m_current_heading);

    const float dot_value = math::Dot(delta, vector_angle);
    const float cross_value = math::Cross(delta, vector_angle);

    const float scaled_clamped_dot = math::Scale01Clamped(dot_value, 1.0f, 0.5f);
    const float scale_value = scaled_clamped_dot * ((cross_value < 0.0f) ? 1.0f : -1.0f);

    const float radians_turn = update_context.delta_s * math::ToRadians(tweak_values::degrees_per_second);
    const float turn_value = scale_value * radians_turn;

    m_current_heading += turn_value;

    const math::Vector angle1 = math::VectorFromAngle(m_current_heading);

    //m_body->SetVelocity(angle1 * tweak_values::velocity);
    m_body->ApplyLocalImpulse(angle1 * tweak_values::velocity, math::ZeroVec);
    //m_body->ApplyLocalForce(angle1 * tweak_values::velocity, math::ZeroVec);

    const float distance = math::DistanceBetween(target_position, entity_position);
    if(distance > tweak_values::trigger_distance)
        m_states.TransitionTo(States::SLEEPING);
}
