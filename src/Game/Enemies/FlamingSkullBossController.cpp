
#include "FlamingSkullBossController.h"

#include "CollisionConfiguration.h"
#include "DamageSystem/DamageSystem.h"
#include "Entity/TargetSystem.h"
#include "Navigation/NavigationSystem.h"
#include "DamageSystem/Shockwave.h"
#include "Debug/IDebugDrawer.h"
#include "Weapons/WeaponTypes.h"

#include "Physics/IBody.h"
#include "Physics/PhysicsSystem.h"

#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"


namespace tweak_values
{
    constexpr float trigger_distance = 5.0f;
    constexpr float time_before_hunt_s = 0.3f;
    constexpr float visibility_check_interval_s = 1.0f;
    constexpr uint32_t collision_damage = 25;
    constexpr float shockwave_radius = 2.0f;
    constexpr float shockwave_magnitude = 5.0f;

    constexpr float degrees_per_second = 180.0f;
    constexpr float velocity = 1.0f;
}

using namespace game;

FlamingSkullBossController::FlamingSkullBossController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_entity_id(entity_id)
    , m_awake_state_timer_s(0.0f)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_damage_system = system_context->GetSystem<game::DamageSystem>();
    m_navigation_system = system_context->GetSystem<game::NavigationSystem>();
    m_target_system = system_context->GetSystem<game::TargetSystem>();

    m_transform = &m_transform_system->GetTransform(entity_id);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    mono::IBody* body = m_physics_system->GetBody(entity_id);
    body->AddCollisionHandler(this);

    m_homing_movement.SetBody(body);
    m_homing_movement.SetForwardVelocity(tweak_values::velocity);
    m_homing_movement.SetAngularVelocity(tweak_values::degrees_per_second);

    m_tracking_movement.Init(body, m_navigation_system);
    m_tracking_movement.SetTrackingSpeed(tweak_values::velocity);

    m_stagger_behaviour.SetChanceAndDuration(0.1f, 1.0f);

    using namespace std::placeholders;

    const MyStateMachine::StateTable& state_table = {
        MyStateMachine::MakeState(States::SLEEPING, &FlamingSkullBossController::ToSleep, &FlamingSkullBossController::SleepState, this),
        MyStateMachine::MakeState(States::AWAKE,    &FlamingSkullBossController::ToAwake, &FlamingSkullBossController::AwakeState, this),
        MyStateMachine::MakeState(States::TRACKING, &FlamingSkullBossController::ToTracking, &FlamingSkullBossController::TrackingState, this),
        MyStateMachine::MakeState(States::HUNT,     &FlamingSkullBossController::ToHunt,  &FlamingSkullBossController::HuntState, this),
    };
    m_states.SetStateTableAndState(state_table, States::SLEEPING);
}

FlamingSkullBossController::~FlamingSkullBossController()
{ }

void FlamingSkullBossController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
    m_stagger_behaviour.Update(update_context);
}

void FlamingSkullBossController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    debug_drawer->DrawCircle(world_position, tweak_values::trigger_distance, mono::Color::MAGENTA);
}

const char* FlamingSkullBossController::GetDebugCategory() const
{
    return "Flaming Skull Boss";
}

mono::CollisionResolve FlamingSkullBossController::OnCollideWith(
    mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t category)
{
    if(m_states.ActiveState() == States::SLEEPING)
        m_visibility_check_timer_s = tweak_values::visibility_check_interval_s;

    if(category == CollisionCategory::PLAYER)
    {
        const math::Vector& entity_position = math::GetPosition(*m_transform);
        game::ShockwaveAt(m_physics_system, entity_position, tweak_values::shockwave_radius, tweak_values::shockwave_magnitude);

        const uint32_t other_entity_id = mono::PhysicsSystem::GetIdFromBody(body);
        m_damage_system->ApplyDamage(other_entity_id, m_entity_id, NO_WEAPON_IDENTIFIER, DamageDetails(tweak_values::collision_damage, false, false));
    }
    else if(category == CollisionCategory::PLAYER_BULLET)
    {
        m_stagger_behaviour.TestForStaggering();
    }

    return mono::CollisionResolve::NORMAL;
}

void FlamingSkullBossController::OnSeparateFrom(mono::IBody* body)
{ }

void FlamingSkullBossController::ToSleep()
{
    m_visibility_check_timer_s = 0.0f;
}

void FlamingSkullBossController::SleepState(const mono::UpdateContext& update_context)
{
    m_visibility_check_timer_s += update_context.delta_s;
    if(m_visibility_check_timer_s < tweak_values::visibility_check_interval_s)
        return;

    m_visibility_check_timer_s = 0.0f;
    
    const math::Vector& entity_position = math::GetPosition(*m_transform);

    m_aquired_target = m_target_system->AquireTarget(TargetFaction::Player, entity_position, tweak_values::trigger_distance);
    if(!m_aquired_target->IsValid())
        return;

    const bool within_engage_range = m_aquired_target->IsWithinRange(entity_position, tweak_values::trigger_distance);
    if(within_engage_range)
    {
        const bool sees_target = m_target_system->SeesTarget(m_entity_id, m_aquired_target.get());
        if(sees_target)
            m_states.TransitionTo(States::AWAKE);
    }
    else
    {
        m_states.TransitionTo(States::TRACKING);
    }
}

void FlamingSkullBossController::ToAwake()
{
    m_awake_state_timer_s = 0.0f;

    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::SLEEPING);
        return;
    }

    const math::Vector& entity_position = math::GetPosition(*m_transform);
    const math::Vector delta = (entity_position - m_aquired_target->Position());
    const float angle = math::AngleFromVector(delta);
    m_homing_movement.SetHeading(angle);
}

void FlamingSkullBossController::AwakeState(const mono::UpdateContext& update_context)
{
    m_awake_state_timer_s += update_context.delta_s;
    if(m_awake_state_timer_s < tweak_values::time_before_hunt_s)
        return;

    m_states.TransitionTo(States::HUNT);
}

void FlamingSkullBossController::ToTracking()
{
}

void FlamingSkullBossController::TrackingState(const mono::UpdateContext& update_context)
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::SLEEPING);
        return;
    }

    const TrackingResult result = m_tracking_movement.Run(update_context, m_aquired_target->Position());
    switch(result.state)
    {
    case TrackingState::NO_PATH:
        m_states.TransitionTo(States::SLEEPING);
        break;
 
    case TrackingState::TRACKING:
        if(result.distance_to_target < (tweak_values::trigger_distance - 1.0f))
            m_states.TransitionTo(States::HUNT);
        break;

    case TrackingState::AT_TARGET:
        m_states.TransitionTo(States::HUNT);
        break;
    }
}

void FlamingSkullBossController::ToHunt()
{
    if(!m_aquired_target->IsValid())
        m_states.TransitionTo(States::SLEEPING);
}

void FlamingSkullBossController::HuntState(const mono::UpdateContext& update_context)
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::SLEEPING);
        return;
    }

    const bool in_stagger = m_stagger_behaviour.IsStaggering();
    if(in_stagger)
        return;

    m_homing_movement.SetTargetPosition(m_aquired_target->Position());
    const game::HomingResult result = m_homing_movement.Run(update_context);
    const math::Vector new_direction = math::VectorFromAngle(result.new_heading);

    if(new_direction.x < 0.0f)
        m_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    else
        m_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

    if(result.distance_to_target > tweak_values::trigger_distance)
        m_states.TransitionTo(States::SLEEPING);
}
