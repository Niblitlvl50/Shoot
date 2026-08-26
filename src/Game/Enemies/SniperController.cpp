
#include "SniperController.h"

#include "Math/MathFunctions.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/IBody.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "Rendering/Color.h"
#include "TransformSystem/TransformSystem.h"
#include "Debug/IDebugDrawer.h"
#include "SystemContext.h"
#include "Util/Random.h"

#include "Entity/TargetSystem.h"
#include "Navigation/NavigationSystem.h"
#include "Weapons/IWeapon.h"
#include "Weapons/WeaponSystem.h"

#include <cmath>

namespace tweak_values
{
    constexpr float activate_distance = 8.0f;
    constexpr float preferred_distance = 4.0f;   // ideal firing range
    constexpr float danger_distance = 3.0f;       // retreat if player closes to this
    constexpr float reposition_tolerance = 1.0f;  // close enough to preferred range
    constexpr float aim_duration_s = 2.2f;        // telegraph before firing
    constexpr float post_fire_delay_s = 1.2f;     // brief pause after shot before repositioning
    constexpr float move_speed = 1.2f;
    constexpr float retreat_speed = 1.8f;
    constexpr float degrees_per_second = 300.0f;
    constexpr float reposition_jitter_deg = 34.0f; // random angle offset when picking new position
}

using namespace game;

SniperController::SniperController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_entity_id(entity_id)
    , m_aim_timer_s(0.0f)
    , m_fire_timer_s(0.0f)
    , m_reposition_from_search(false)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_target_system = system_context->GetSystem<game::TargetSystem>();
    m_navigation_system = system_context->GetSystem<game::NavigationSystem>();

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
    m_run_anim_id  = m_sprite->GetAnimationIdFromName("walk");

    game::WeaponSystem* weapon_system = system_context->GetSystem<game::WeaponSystem>();
    m_weapon = weapon_system->CreatePrimaryWeapon(entity_id, WeaponFaction::ENEMY);

    const SniperStateMachine::StateTable state_table = {
        SniperStateMachine::MakeState(States::IDLE,       &SniperController::ToIdle,       &SniperController::Idle,       this),
        SniperStateMachine::MakeState(States::REPOSITION, &SniperController::ToReposition, &SniperController::Reposition, this),
        SniperStateMachine::MakeState(States::RETREAT,    &SniperController::ToRetreat,    &SniperController::Retreat,    this),
        SniperStateMachine::MakeState(States::SEARCH,     &SniperController::ToSearch,     &SniperController::Search,     this),
        SniperStateMachine::MakeState(States::AIM,        &SniperController::ToAim,        &SniperController::Aim,        this),
        SniperStateMachine::MakeState(States::FIRE,       &SniperController::ToFire,       &SniperController::Fire,       this),
    };
    m_states.SetStateTableAndState(state_table, States::IDLE);
}

void SniperController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
    m_weapon->UpdateWeaponState(update_context.timestamp);
}

void SniperController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    debug_drawer->DrawCircle(world_position, tweak_values::activate_distance, mono::Color::GREEN);
    debug_drawer->DrawCircle(world_position, tweak_values::preferred_distance, mono::Color::CYAN);
    debug_drawer->DrawCircle(world_position, tweak_values::danger_distance, mono::Color::RED);

    const char* state_string = nullptr;
    switch(m_states.ActiveState())
    {
    case States::IDLE:       state_string = "Idle";       break;
    case States::REPOSITION: state_string = "Reposition"; break;
    case States::RETREAT:    state_string = "Retreat";    break;
    case States::SEARCH:     state_string = "Search";     break;
    case States::AIM:        state_string = "Aim";        break;
    case States::FIRE:       state_string = "Fire";       break;
    }
    debug_drawer->DrawWorldText(state_string, world_position, mono::Color::OFF_WHITE);

    if(m_aquired_target && m_aquired_target->IsValid())
        debug_drawer->DrawLine({ world_position, m_aquired_target->Position() }, 1.0f, mono::Color::CYAN);
}

const char* SniperController::GetDebugCategory() const
{
    return "Sniper";
}

void SniperController::ToIdle()
{
    m_sprite->SetShade(mono::Color::WHITE);
    m_sprite->SetAnimation(m_idle_anim_id);
}

void SniperController::Idle(const mono::UpdateContext& update_context)
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    m_aquired_target = m_target_system->AquireTarget(TargetFaction::Player, world_position, tweak_values::activate_distance);

    if(m_aquired_target->IsValid())
        m_states.TransitionTo(States::REPOSITION);
}

static math::Vector CalcRepositionTarget(
    const math::Vector& self_pos,
    const math::Vector& player_pos,
    float preferred_distance)
{
    const math::Vector away = math::Normalized(self_pos - player_pos);
    const float jitter = mono::Random(-math::ToRadians(tweak_values::reposition_jitter_deg), math::ToRadians(tweak_values::reposition_jitter_deg));
    const float c = std::cos(jitter);
    const float s = std::sin(jitter);
    const math::Vector jittered = { away.x * c - away.y * s, away.x * s + away.y * c };
    return player_pos + jittered * preferred_distance;
}

void SniperController::ToReposition()
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector player_pos = m_aquired_target->Position();

    math::Vector target;
    if(m_reposition_from_search)
    {
        // Rotate 90° (random left or right) to explore around the obstacle.
        const math::Vector to_self = math::Normalized(world_position - player_pos);
        const float current_angle = std::atan2(to_self.y, to_self.x);
        const float side = mono::Chance(50) ? 1.0f : -1.0f;
        const float new_angle = current_angle + side * math::ToRadians(90.0f);
        target = player_pos + math::Vector(std::cos(new_angle), std::sin(new_angle)) * tweak_values::preferred_distance;
        m_reposition_from_search = false;
    }
    else
    {
        target = CalcRepositionTarget(world_position, player_pos, tweak_values::preferred_distance);
    }

    m_homing_movement.SetForwardVelocity(tweak_values::move_speed);
    m_homing_movement.SetTargetPosition(target);
    m_sprite->SetAnimation(m_run_anim_id);
}

void SniperController::Reposition(const mono::UpdateContext& update_context)
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);

    if(m_aquired_target->IsWithinDistance(world_position, tweak_values::danger_distance)
        && m_target_system->SeesTarget(m_entity_id, m_aquired_target.get()))
    {
        m_states.TransitionTo(States::RETREAT);
        return;
    }

    const HomingResult result = m_homing_movement.Run(update_context);

    if(result.distance_to_target < tweak_values::reposition_tolerance)
    {
        if(m_target_system->SeesTarget(m_entity_id, m_aquired_target.get()))
            m_states.TransitionTo(States::AIM);
        else
            m_states.TransitionTo(States::SEARCH);
    }
    else if(result.is_stuck)
    {
        m_states.TransitionTo(States::SEARCH);
    }
}

void SniperController::ToRetreat()
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector away = math::Normalized(world_position - m_aquired_target->Position());
    const float side = mono::Chance(50) ? 1.0f : -1.0f;
    const math::Vector retreat_target = world_position + away * (tweak_values::preferred_distance * 0.7f)
        + math::Perpendicular(away) * side * 2.0f;

    m_homing_movement.SetForwardVelocity(tweak_values::retreat_speed);
    m_homing_movement.SetTargetPosition(retreat_target);
    m_sprite->SetShade(mono::Color::WHITE);
    m_sprite->SetAnimation(m_run_anim_id);
}

void SniperController::Retreat(const mono::UpdateContext& update_context)
{
    const HomingResult result = m_homing_movement.Run(update_context);

    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    const bool safe = !m_aquired_target->IsValid() ||
        !m_aquired_target->IsWithinDistance(world_position, tweak_values::danger_distance * 1.5f);

    if(result.distance_to_target < 0.3f || safe)
        m_states.TransitionTo(States::REPOSITION);
}

void SniperController::ToSearch()
{
    m_sprite->SetAnimation(m_run_anim_id);
}

void SniperController::Search(const mono::UpdateContext& update_context)
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    if(m_target_system->SeesTarget(m_entity_id, m_aquired_target.get()))
    {
        m_states.TransitionTo(States::AIM);
        return;
    }

    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    const float distance_to_player = math::DistanceBetween(world_position, m_aquired_target->Position());

    // Stop at preferred_distance — close enough to check LOS from a new angle, but never close
    // enough to trigger retreat.
    if(distance_to_player <= tweak_values::preferred_distance)
    {
        m_reposition_from_search = true;
        m_states.TransitionTo(States::REPOSITION);
        return;
    }

    m_tracking_movement.Run(update_context, m_aquired_target->Position());
}

void SniperController::ToAim()
{
    m_aim_timer_s = 0.0f;
    m_reposition_from_search = false;
    m_sprite->SetShade(mono::Color::RGBA(1.0f, 0.2f, 0.2f, 1.0f));
    m_sprite->SetAnimation(m_idle_anim_id);
}

void SniperController::Aim(const mono::UpdateContext& update_context)
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);

    if(m_aquired_target->IsWithinDistance(world_position, tweak_values::danger_distance)
        && m_target_system->SeesTarget(m_entity_id, m_aquired_target.get()))
    {
        m_states.TransitionTo(States::RETREAT);
        return;
    }

    if(!m_target_system->SeesTarget(m_entity_id, m_aquired_target.get()))
    {
        m_states.TransitionTo(States::REPOSITION);
        return;
    }

    // Flip sprite to face target while aiming.
    const bool is_left = world_position.x < m_aquired_target->Position().x;
    if(is_left)
        m_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    else
        m_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

    m_aim_timer_s += update_context.delta_s;
    if(m_aim_timer_s >= tweak_values::aim_duration_s)
        m_states.TransitionTo(States::FIRE);
}

void SniperController::ToFire()
{
    m_fire_timer_s = 0.0f;
}

void SniperController::Fire(const mono::UpdateContext& update_context)
{
    if(m_fire_timer_s == 0.0f)
    {
        if(m_aquired_target->IsValid())
        {
            const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
            const WeaponState state = m_weapon->Fire(world_position, m_aquired_target->Position(), update_context.timestamp);
            if(state == WeaponState::OUT_OF_AMMO)
                m_weapon->Reload(update_context.timestamp);
        }
        m_sprite->SetShade(mono::Color::WHITE);
    }

    m_fire_timer_s += update_context.delta_s;
    if(m_fire_timer_s >= tweak_values::post_fire_delay_s)
        m_states.TransitionTo(States::REPOSITION);
}
