
#include "GoblinFireController.h"

#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "Math/MathFunctions.h"
#include "Math/EasingFunctions.h"
#include "Util/Random.h"
#include "Debug/IDebugDrawer.h"

#include "Entity/TargetSystem.h"
#include "Navigation/NavigationSystem.h"
#include "Player/PlayerInfo.h"
#include "SystemContext.h"
#include "Weapons/IWeapon.h"
#include "Weapons/WeaponSystem.h"


namespace tweak_values
{
    constexpr float activate_distance_to_player_threshold = 6.0f;
    constexpr float distance_to_player_threshold = 3.0f;
    constexpr float move_speed = 0.5f;
    constexpr float degrees_per_second = 360.0f;

    constexpr float idle_timer_s = 1.0f;
    constexpr float attack_start_delay_s = 0.5f;
    constexpr float attack_sequence_delay_s = 0.05f;
    constexpr uint32_t n_attacks = 3;
}

using namespace game;

GoblinFireController::GoblinFireController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_entity_id(entity_id)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_navigation_system = system_context->GetSystem<NavigationSystem>();
    m_target_system = system_context->GetSystem<TargetSystem>();

    mono::PhysicsSystem* physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    mono::IBody* body = physics_system->GetBody(entity_id);

    m_homing_movement.SetBody(body);
    m_homing_movement.SetForwardVelocity(tweak_values::move_speed);
    m_homing_movement.SetAngularVelocity(tweak_values::degrees_per_second);

    m_tracking_movement.Init(body, m_navigation_system);
    m_tracking_movement.SetTrackingSpeed(tweak_values::move_speed);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    m_idle_anim_id = m_sprite->GetAnimationIdFromName("idle");
    m_run_anim_id = m_sprite->GetAnimationIdFromName("walk");

    game::WeaponSystem* weapon_system = system_context->GetSystem<game::WeaponSystem>();
    m_weapon = weapon_system->CreatePrimaryWeapon(entity_id, WeaponFaction::ENEMY);

    const GoblinStateMachine::StateTable state_table = {
        GoblinStateMachine::MakeState(States::IDLE, &GoblinFireController::ToIdle, &GoblinFireController::Idle, this),
        GoblinStateMachine::MakeState(States::TRACKING, &GoblinFireController::ToTracking, &GoblinFireController::Tracking, this),
        GoblinStateMachine::MakeState(States::REPOSITION, &GoblinFireController::ToReposition, &GoblinFireController::Reposition, this),
        GoblinStateMachine::MakeState(States::PREPARE_ATTACK, &GoblinFireController::ToPrepareAttack, &GoblinFireController::PrepareAttack, this),
        GoblinStateMachine::MakeState(States::ATTACKING, &GoblinFireController::ToAttacking, &GoblinFireController::Attacking, this),
    };

    m_states.SetStateTableAndState(state_table, States::IDLE);
}

void GoblinFireController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

void GoblinFireController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector& homing_target_position = m_homing_movement.GetTargetPosition();

    debug_drawer->DrawCircle(world_position, tweak_values::activate_distance_to_player_threshold, mono::Color::CYAN);
    debug_drawer->DrawCircle(world_position, tweak_values::distance_to_player_threshold, mono::Color::CYAN);

    debug_drawer->DrawLine({ world_position, homing_target_position }, 1.0f, mono::Color::BLUE);
    debug_drawer->DrawPoint(homing_target_position, 5.0f, mono::Color::BLUE);

    const char* state_string = nullptr;
    switch(m_states.ActiveState())
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
    case States::PREPARE_ATTACK:
        state_string = "Prepare Attack";
        break;
    case States::ATTACKING:
        state_string = "Attacking";
        break;
    }

    debug_drawer->DrawWorldText(state_string, world_position, mono::Color::OFF_WHITE);
}

const char* GoblinFireController::GetDebugCategory() const
{
    return "Goblin Fire";
}

void GoblinFireController::ToIdle()
{
    m_idle_timer_s = 0.0f;
    m_sprite->SetShade(mono::Color::WHITE);
    m_sprite->SetAnimation(m_idle_anim_id);
}

void GoblinFireController::Idle(const mono::UpdateContext& update_context)
{
    m_idle_timer_s += update_context.delta_s;

    if(m_idle_timer_s < tweak_values::idle_timer_s)
        return;

    const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);
    m_aquired_target = m_target_system->AquireTarget(TargetFaction::Player, world_position, tweak_values::activate_distance_to_player_threshold);

    if(!m_aquired_target->IsValid())
    {
        m_idle_timer_s = 0.0f;
        return;
    }

    const math::Vector& target_position = m_aquired_target->Position();

    const bool is_left_of = (target_position.x < world_position.x);
    if(is_left_of)
        m_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    else 
        m_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

    const bool is_within_range = m_aquired_target->IsWithinRange(world_position, tweak_values::activate_distance_to_player_threshold);
    if(is_within_range)
    {
        const bool transition_to_attack =
            mono::Chance(25) && m_target_system->SeesTarget(m_entity_id, m_aquired_target.get());
        const States new_state = transition_to_attack ? States::PREPARE_ATTACK : States::REPOSITION;
        m_states.TransitionTo(new_state);
    }
    else
    {
        m_states.TransitionTo(States::TRACKING);
    }

    m_idle_timer_s = 0.0f;
}

void GoblinFireController::ToTracking()
{
    m_sprite->SetAnimation(m_run_anim_id);
}

void GoblinFireController::Tracking(const mono::UpdateContext& update_context)
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    const TrackingResult result = m_tracking_movement.Run(update_context, m_aquired_target->Position());
    switch(result.state)
    {
    case TrackingState::NO_PATH:
        m_states.TransitionTo(States::IDLE);
        break;
 
    case TrackingState::TRACKING:
        if(result.distance_to_target < (tweak_values::activate_distance_to_player_threshold - 1.0f))
            m_states.TransitionTo(States::REPOSITION);
        break;

    case TrackingState::AT_TARGET:
        m_states.TransitionTo(States::REPOSITION);
        break;
    }
}

void GoblinFireController::ToReposition()
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

    if(distance_to_target < tweak_values::distance_to_player_threshold)
    {
        const float multiplier = mono::Chance(50) ? -1.0f : 1.0f;
        homing_target += math::Perpendicular(halfway_delta) * multiplier; // Move sideways
    }
    else
    {
        homing_target += halfway_delta;
    }

    m_sprite->SetAnimation(m_run_anim_id);
    m_homing_movement.SetTargetPosition(homing_target);
}

void GoblinFireController::Reposition(const mono::UpdateContext& update_context)
{
    const game::HomingResult result = m_homing_movement.Run(update_context);
    if(result.distance_to_target < 0.1f)
    {
        const bool transition_to_attack =
            m_aquired_target->IsValid() &&
            mono::Chance(75) &&
            m_target_system->SeesTarget(m_entity_id, m_aquired_target.get());

        const States new_state = transition_to_attack ? States::PREPARE_ATTACK : States::IDLE;
        m_states.TransitionTo(new_state);
    }
    else if(result.is_stuck)
    {
        m_states.TransitionTo(States::IDLE);
    }
}

void GoblinFireController::ToPrepareAttack()
{
    m_prepare_timer_s = 0.0f;
    m_sprite->SetShade(mono::Color::RED);
}

void GoblinFireController::PrepareAttack(const mono::UpdateContext& update_context)
{
    m_prepare_timer_s += update_context.delta_s;
    if(m_prepare_timer_s > tweak_values::attack_start_delay_s)
        m_states.TransitionTo(States::ATTACKING);
}

void GoblinFireController::ToAttacking()
{
    m_attack_timer_s = 0.0f;
    m_n_attacks = 0;
}

void GoblinFireController::Attacking(const mono::UpdateContext& update_context)
{
    if(m_n_attacks >= tweak_values::n_attacks)
        m_states.TransitionTo(States::IDLE);

    if(m_weapon->UpdateWeaponState(update_context.timestamp) == game::WeaponState::RELOADING)
        m_states.TransitionTo(States::IDLE);

    if(!m_aquired_target->IsValid())
        m_states.TransitionTo(States::IDLE);

    m_attack_timer_s += update_context.delta_s;
    if(m_attack_timer_s > tweak_values::attack_sequence_delay_s)
    {
        const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);

        game::WeaponState fire_state = m_weapon->Fire(world_position, m_aquired_target->Position(), update_context.timestamp);
        if(fire_state == game::WeaponState::FIRE)
            m_n_attacks++;
        else if(fire_state == game::WeaponState::OUT_OF_AMMO)
            m_weapon->Reload(update_context.timestamp);
    
        m_attack_timer_s = 0.0f;
    }
}
