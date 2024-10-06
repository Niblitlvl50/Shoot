
#include "FlyingMonsterController.h"

#include "Behaviour/TrackingBehaviour.h"
#include "Debug/IDebugDrawer.h"
#include "Entity/TargetSystem.h"
#include "Weapons/IWeapon.h"
#include "Weapons/WeaponSystem.h"
#include "Navigation/NavigationSystem.h"

#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteProperties.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Util/Random.h"

#include <cmath>

namespace tweak_values
{
    constexpr float idle_time_s = 1.0f;
    constexpr float attack_start_delay_s = 0.25f;

    constexpr float move_speed = 0.5f;
    constexpr float degrees_per_second = 360.0f;

    constexpr float attack_distance = 2.5f;
    constexpr float max_attack_distance = 3.0f;
    constexpr float track_to_player_distance = 4.0f;
    constexpr float loose_interest_distance = 5.0f;

    constexpr int bullets_to_emit = 2;
}

using namespace game;

FlyingMonsterController::FlyingMonsterController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_entity_id(entity_id)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();

    game::NavigationSystem* navigation_system = system_context->GetSystem<game::NavigationSystem>();

    game::WeaponSystem* weapon_system = system_context->GetSystem<game::WeaponSystem>();
    m_weapon = weapon_system->CreatePrimaryWeapon(entity_id, WeaponFaction::ENEMY);

    mono::PhysicsSystem* physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    mono::IBody* entity_body = physics_system->GetBody(entity_id);
    m_tracking_movement.Init(entity_body, navigation_system);

    m_homing_movement.SetBody(entity_body);
    m_homing_movement.SetForwardVelocity(tweak_values::move_speed);
    m_homing_movement.SetAngularVelocity(tweak_values::degrees_per_second);

    m_target_system = system_context->GetSystem<TargetSystem>();

    const FlyingMonsterStateMachine::StateTable& state_table = {
        FlyingMonsterStateMachine::MakeState(States::IDLE, &FlyingMonsterController::ToIdle, &FlyingMonsterController::Idle, this),
        FlyingMonsterStateMachine::MakeState(States::TRACKING, &FlyingMonsterController::ToTracking, &FlyingMonsterController::Tracking, this),
        FlyingMonsterStateMachine::MakeState(States::REPOSITION, &FlyingMonsterController::ToReposition, &FlyingMonsterController::Reposition, this),
        FlyingMonsterStateMachine::MakeState(States::ATTACK_ANTICIPATION, &FlyingMonsterController::ToAttackAnticipation, &FlyingMonsterController::AttackAnticipation, this),
        FlyingMonsterStateMachine::MakeState(States::ATTACKING, &FlyingMonsterController::ToAttacking, &FlyingMonsterController::Attacking, this),
    };
    m_states.SetStateTableAndState(state_table, States::IDLE);
}

FlyingMonsterController::~FlyingMonsterController()
{ }

void FlyingMonsterController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
    m_weapon->UpdateWeaponState(update_context.timestamp);

    if(m_aquired_target && m_aquired_target->IsValid())
    {
        const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);
        const math::Vector& target_world_position = m_aquired_target->Position();
        const bool is_left_of_target = world_position.x < target_world_position.x;

        mono::ISprite* sprite = m_sprite_system->GetSprite(m_entity_id);
        if(is_left_of_target)
            sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
        else
            sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    }
}

void FlyingMonsterController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const States active_state = m_states.ActiveState();
    const char* state_string = nullptr;

    switch(active_state)
    {
    case States::IDLE:
        state_string = "Idle";
        break;
    case States::TRACKING:
        state_string = "Tracking";
        break;
    case States::REPOSITION:
        state_string = "Reposition";
        break;
    case States::ATTACK_ANTICIPATION:
        state_string = "Attack Anticipation";
        break;
    case States::ATTACKING:
        state_string = "Attacking";
        break;
    }

    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    debug_drawer->DrawWorldText(state_string, world_position, mono::Color::OFF_WHITE);
    debug_drawer->DrawCircle(world_position, tweak_values::track_to_player_distance, mono::Color::CYAN);
    debug_drawer->DrawCircle(world_position, tweak_values::attack_distance, mono::Color::RED);
    debug_drawer->DrawCircle(world_position, tweak_values::max_attack_distance, mono::Color::RED);
    debug_drawer->DrawCircle(world_position, tweak_values::loose_interest_distance, mono::Color::GREEN);

    if(active_state == States::TRACKING)
    {
        const math::Vector& tracking_position = m_tracking_movement.GetTrackingPosition();
        debug_drawer->DrawLine({ world_position, tracking_position }, 1.0f, mono::Color::BLUE);
        debug_drawer->DrawPoint(tracking_position, 10.0f, mono::Color::RED);
    }

    if(m_aquired_target && m_aquired_target->IsValid())
    {
        const math::Vector& target_position = m_aquired_target->Position();
        debug_drawer->DrawLine({ world_position, target_position }, 1.0f, mono::Color::CYAN);
        debug_drawer->DrawCircle(target_position, 0.5f, mono::Color::CYAN);
    }
}

const char* FlyingMonsterController::GetDebugCategory() const
{
    return "Flying Monster";
}

void FlyingMonsterController::ToIdle()
{
    m_idle_timer_s = 0.0f;

    mono::ISprite* sprite = m_sprite_system->GetSprite(m_entity_id);
    sprite->SetShade(mono::Color::WHITE);
}

void FlyingMonsterController::Idle(const mono::UpdateContext& update_context)
{
    m_idle_timer_s += update_context.delta_s;
    if(m_idle_timer_s < tweak_values::idle_time_s)
        return;

    m_idle_timer_s = 0.0f;
    
    const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);
    m_aquired_target = m_target_system->AquireTarget(TargetFaction::Player, world_position, tweak_values::track_to_player_distance);

    if(!m_aquired_target->IsValid())
        return;

    const bool sees_target = m_target_system->SeesTarget(m_entity_id, m_aquired_target.get());
    if(!sees_target)
        return;

    const float distance_to_player = math::DistanceBetween(world_position, m_aquired_target->Position());
    if(distance_to_player < tweak_values::attack_distance)
    {
        m_states.TransitionTo(States::REPOSITION);
    }
    else if(distance_to_player < tweak_values::max_attack_distance)
    {
        const bool reposition = mono::Chance(50);
        m_states.TransitionTo(reposition ? States::REPOSITION : States::ATTACK_ANTICIPATION);
    }
    else if(distance_to_player < tweak_values::track_to_player_distance)
    {
        m_states.TransitionTo(States::TRACKING);
    }
}

void FlyingMonsterController::ToTracking()
{ }

void FlyingMonsterController::Tracking(const mono::UpdateContext& update_context)
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector& target_world_position = m_aquired_target->Position();

    const float distance_to_player = math::DistanceBetween(world_position, target_world_position);
    if(distance_to_player > tweak_values::loose_interest_distance)
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    if(distance_to_player < tweak_values::attack_distance)
    {
        const bool sees_player = m_target_system->SeesTarget(m_entity_id, m_aquired_target.get());
        if(sees_player)
        {
            m_states.TransitionTo(States::ATTACK_ANTICIPATION);
            return;
        }
    }

    const TrackingResult result = m_tracking_movement.Run(update_context, target_world_position);
    if(result.state == TrackingState::NO_PATH || result.state == TrackingState::AT_TARGET)
        m_states.TransitionTo(States::IDLE);
}

void FlyingMonsterController::ToReposition()
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector& target_position = m_transform_system->GetWorldPosition(m_aquired_target->TargetId());

    const math::Vector delta = target_position - world_position;
    const float distance_to_target = math::Length(delta);

    math::Vector homing_target = world_position;

    // Move towards player
    const math::Vector halfway_delta = delta / 2.0f;

    if(distance_to_target < tweak_values::attack_distance)
    {
        const float multiplier = mono::Chance(50) ? -1.0f : 1.0f;
        homing_target += math::Perpendicular(halfway_delta) * multiplier; // Move sideways
    }
    else
    {
        homing_target += halfway_delta;
    }

    m_homing_movement.SetTargetPosition(homing_target);
}

void FlyingMonsterController::Reposition(const mono::UpdateContext& update_context)
{
    const game::HomingResult result = m_homing_movement.Run(update_context);
    if(result.distance_to_target < 0.1f)
    {
        const bool transition_to_attack =
            m_aquired_target->IsValid() &&
            mono::Chance(75) &&
            m_target_system->SeesTarget(m_entity_id, m_aquired_target.get());

        const States new_state = transition_to_attack ? States::ATTACK_ANTICIPATION : States::IDLE;
        m_states.TransitionTo(new_state);
    }
    else if(result.is_stuck)
    {
        m_states.TransitionTo(States::IDLE);
    }
}

void FlyingMonsterController::ToAttackAnticipation()
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector& target_world_position = m_aquired_target->Position();

    const float distance_to_target = math::DistanceBetween(world_position, target_world_position);
    if(distance_to_target > tweak_values::max_attack_distance)
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    mono::ISprite* sprite = m_sprite_system->GetSprite(m_entity_id);
    sprite->SetShade(mono::Color::RGBA(1.0f, 0.25f, 0.25f));

    m_attack_anticipation_timer_s = 0.0f;
}

void FlyingMonsterController::AttackAnticipation(const mono::UpdateContext& update_context)
{
    m_attack_anticipation_timer_s += update_context.delta_s;
    if(m_attack_anticipation_timer_s > tweak_values::attack_start_delay_s)
        m_states.TransitionTo(States::ATTACKING);
}

void FlyingMonsterController::ToAttacking()
{
    m_bullets_fired = 0;
}

void FlyingMonsterController::Attacking(const mono::UpdateContext& update_context)
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    if(m_bullets_fired < tweak_values::bullets_to_emit)
    {
        const math::Vector& position = m_transform_system->GetWorldPosition(m_entity_id);
        const game::WeaponState fire_state = m_weapon->Fire(position, m_aquired_target->Position(), update_context.timestamp);
        if(fire_state == game::WeaponState::FIRE)
            m_bullets_fired++;
        else if(fire_state == game::WeaponState::OUT_OF_AMMO)
            m_weapon->Reload(update_context.timestamp);
    }
    else
    {
        m_states.TransitionTo(States::IDLE);
    }
}
