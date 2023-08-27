
#include "EyeMonsterController.h"

#include "CollisionConfiguration.h"
#include "DamageSystem/DamageSystem.h"
#include "Shockwave.h"
#include "Debug/IDebugDrawer.h"
#include "Entity/TargetSystem.h"
#include "Navigation/NavigationSystem.h"

#include "Physics/IBody.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsSpace.h"

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
    constexpr float engage_distance = 4.0f;
    constexpr float disengage_distance = 5.0f;

    constexpr float time_before_hunt_s = 0.3f;
    constexpr float visibility_check_interval_s = 1.0f;
    constexpr float retarget_delay_s = 0.5f;

    constexpr uint32_t collision_damage = 25;

    constexpr float shockwave_radius = 1.0f;
    constexpr float shockwave_magnitude = 5.0f;

    constexpr float degrees_per_second = 180.0f;
    constexpr float velocity_m_per_s = 1.0f;
}

using namespace game;

EyeMonsterController::EyeMonsterController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_entity_id(entity_id)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_navigation_system = system_context->GetSystem<game::NavigationSystem>();

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    mono::IBody* body = m_physics_system->GetBody(entity_id);
    body->AddCollisionHandler(this);

    m_homing_movement.SetBody(body);
    m_homing_movement.SetForwardVelocity(tweak_values::velocity_m_per_s);
    m_homing_movement.SetAngularVelocity(tweak_values::degrees_per_second);

    m_entity_manager = system_context->GetSystem<mono::IEntityManager>();
    m_damage_system = system_context->GetSystem<game::DamageSystem>();
    m_target_system = system_context->GetSystem<game::TargetSystem>();

    using namespace std::placeholders;

    const MyStateMachine::StateTable& state_table = {
        MyStateMachine::MakeState(States::SLEEPING, &EyeMonsterController::ToSleep,     &EyeMonsterController::SleepState, this),
        MyStateMachine::MakeState(States::AWAKE,    &EyeMonsterController::ToAwake,     &EyeMonsterController::AwakeState, this),
        MyStateMachine::MakeState(States::RETARGET, &EyeMonsterController::ToRetarget,  &EyeMonsterController::RetargetState, this),
        MyStateMachine::MakeState(States::TRACKING, &EyeMonsterController::ToTracking,  &EyeMonsterController::TrackingState, &EyeMonsterController::ExitTracking, this),
        MyStateMachine::MakeState(States::HUNT,     &EyeMonsterController::ToHunt,      &EyeMonsterController::HuntState, &EyeMonsterController::ExitHunt, this),
    };
    m_states.SetStateTableAndState(state_table, States::SLEEPING);
}

EyeMonsterController::~EyeMonsterController()
{ }

void EyeMonsterController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

void EyeMonsterController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    debug_drawer->DrawCircle(world_position, tweak_values::engage_distance, mono::Color::MAGENTA);
    debug_drawer->DrawCircle(world_position, tweak_values::disengage_distance, mono::Color::CYAN);

    const char* state = "Unknown";
    switch(m_states.ActiveState())
    {
    case States::SLEEPING:
        state = "Sleeping";
        break;
    case States::AWAKE:
        state = "Awake";
        break;
    case States::RETARGET:
        state = "Retarget";
        break;
    case States::TRACKING:
        state = "Tracking";
        break;
    case States::HUNT:
        state = "Hunting";
        break;
    }

    debug_drawer->DrawWorldText(state, world_position, mono::Color::OFF_WHITE);
}

const char* EyeMonsterController::GetDebugCategory() const
{
    return "Flying Eye Monster";
}

mono::CollisionResolve EyeMonsterController::OnCollideWith(
    mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t category)
{
    if(m_states.ActiveState() == States::SLEEPING)
        m_visibility_check_timer_s = tweak_values::visibility_check_interval_s;
        //m_states.TransitionTo(States::AWAKE);

    if(category == CollisionCategory::PLAYER)
    {
        const math::Vector& entity_position = m_transform_system->GetWorldPosition(m_entity_id);
        game::ShockwaveAt(m_physics_system, entity_position, tweak_values::shockwave_radius, tweak_values::shockwave_magnitude);

        const uint32_t other_entity_id = mono::PhysicsSystem::GetIdFromBody(body);
        m_damage_system->ApplyDamage(other_entity_id, tweak_values::collision_damage, m_entity_id);
        m_damage_system->ApplyDamage(m_entity_id, 1000, m_entity_id);
    }

    return mono::CollisionResolve::NORMAL;
}

void EyeMonsterController::OnSeparateFrom(mono::IBody* body)
{ }

void EyeMonsterController::ToSleep()
{
    m_visibility_check_timer_s = 0.0f;
    m_sprite->SetAnimation("close_eye");
}

void EyeMonsterController::SleepState(const mono::UpdateContext& update_context)
{
    m_visibility_check_timer_s += update_context.delta_s;
    if(m_visibility_check_timer_s < tweak_values::visibility_check_interval_s)
        return;

    m_visibility_check_timer_s = 0.0f;

    const math::Vector& entity_position = m_transform_system->GetWorldPosition(m_entity_id);
    m_aquired_target = m_target_system->AquireTarget(entity_position, tweak_values::engage_distance);
    if(m_aquired_target->IsValid())
    {
        const bool sees_target = m_target_system->SeesTarget(m_entity_id, m_aquired_target.get());
        if(sees_target)
            m_states.TransitionTo(States::AWAKE);
        else
            m_states.TransitionTo(States::TRACKING);
    }
}

void EyeMonsterController::ToAwake()
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::SLEEPING);
        return;
    }

    const math::Vector& entity_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector& target_position = m_transform_system->GetWorldPosition(m_aquired_target->TargetId());

    const math::Vector delta = (target_position - entity_position);
    const float angle = math::AngleFromVector(delta);
    m_homing_movement.SetHeading(angle);

    const mono::SpriteAnimationCallback transition_to_hunt = [this](uint32_t sprite_id) {
        m_states.TransitionTo(States::HUNT);
    };
    m_sprite->SetAnimation("open_eye", transition_to_hunt);
}

void EyeMonsterController::AwakeState(const mono::UpdateContext& update_context)
{ }

void EyeMonsterController::ToRetarget()
{
    m_retarget_timer_s = 0.0f;
}

void EyeMonsterController::RetargetState(const mono::UpdateContext& update_context)
{
    m_retarget_timer_s += update_context.delta_s;

    if(m_retarget_timer_s < tweak_values::retarget_delay_s)
        return;

    // Try to aquire a new target
    const math::Vector& entity_position = m_transform_system->GetWorldPosition(m_entity_id);
    m_aquired_target = m_target_system->AquireTarget(entity_position, tweak_values::engage_distance);

    const States new_state = m_aquired_target->IsValid() ? States::HUNT : States::SLEEPING;
    m_states.TransitionTo(new_state);
}

void EyeMonsterController::ToTracking()
{
    mono::IBody* body = m_physics_system->GetBody(m_entity_id);
    m_tracking_movement.Init(body, m_physics_system, m_navigation_system);
    m_tracking_movement.SetTrackingSpeed(0.5f); //tweak_values::velocity_m_per_s);
    m_tracking_movement.UpdateEntityPosition();
}

void EyeMonsterController::TrackingState(const mono::UpdateContext& update_context)
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
        if(result.distance_to_target < (tweak_values::engage_distance - 1.0f))
            m_states.TransitionTo(States::HUNT);
        break;

    case TrackingState::AT_TARGET:
        m_states.TransitionTo(States::HUNT);
        break;
    }
}

void EyeMonsterController::ExitTracking()
{
    m_tracking_movement.Release();
}

void EyeMonsterController::ToHunt()
{
    m_sprite->SetAnimation("idle");

    const math::Vector& entity_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector& target_position = m_transform_system->GetWorldPosition(m_aquired_target->TargetId());

    const math::Vector delta = (target_position - entity_position);
    const float angle = math::AngleFromVector(delta);
    m_homing_movement.SetHeading(angle);
}

void EyeMonsterController::HuntState(const mono::UpdateContext& update_context)
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::RETARGET);
        return;
    }

    const math::Vector& target_entity_position = m_transform_system->GetWorldPosition(m_aquired_target->TargetId());

    m_homing_movement.SetTargetPosition(target_entity_position);
    const game::HomingResult result = m_homing_movement.Run(update_context);
    const math::Vector new_direction = math::VectorFromAngle(result.new_heading);

    if(new_direction.x < 0.0f)
        m_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    else
        m_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

    if(result.distance_to_target > tweak_values::disengage_distance)
        m_states.TransitionTo(States::SLEEPING);
}

void EyeMonsterController::ExitHunt()
{
    m_aquired_target = nullptr;
}