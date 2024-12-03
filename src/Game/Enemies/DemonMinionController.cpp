
#include "DemonMinionController.h"

#include "DamageSystem/DamageSystem.h"
#include "Debug/IDebugDrawer.h"
#include "Entity/TargetSystem.h"
#include "Weapons/IWeapon.h"
#include "Weapons/WeaponSystem.h"
#include "CollisionConfiguration.h"

#include "Math/MathFunctions.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Random.h"

#include "System/System.h"

#include <cmath>

namespace tweak_values
{
    //constexpr float radians_per_second = math::ToRadians(90.0f);
    //constexpr float face_angle = math::ToRadians(5.0f);
    constexpr float activate_distance = 5.0f;
    constexpr float retreat_distance = 3.0f;
    constexpr float advance_distance = 4.5f;

    constexpr float circle_attack_distance = 2.5f;
    constexpr float homing_attack_distance = 4.0f;
    constexpr float long_attack_distance = 6.0f;

    constexpr float circle_attack_cooldown_s = 3.0f;
    constexpr float homing_attack_cooldown_s = 3.0f;
    constexpr float long_attack_cooldown_s = 5.0f;
}

using namespace game;

DemonMinionController::DemonMinionController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_entity_id(entity_id)
    , m_circle_attack_cooldown(0.0f)
    , m_fire_homing_cooldown(0.0f)
    , m_long_attack_cooldown(0.0f)
    , m_beast_mode(false)
{
    game::WeaponSystem* weapon_system = system_context->GetSystem<game::WeaponSystem>();
    m_primary_weapon = weapon_system->CreatePrimaryWeapon(entity_id, WeaponFaction::ENEMY);
    m_secondary_weapon = weapon_system->CreateSecondaryWeapon(entity_id, WeaponFaction::ENEMY);
    m_tertiary_weapon = weapon_system->CreateTertiaryWeapon(entity_id, WeaponFaction::ENEMY);

    m_damage_sound = audio::CreateSound("res/sound/blaster-ricochet.wav", audio::SoundPlayback::ONCE);
    m_death_sound = audio::CreateSound("res/sound/demon_death.wav", audio::SoundPlayback::ONCE);

    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_entity_body = m_physics_system->GetBody(entity_id);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_entity_sprite = sprite_system->GetSprite(entity_id);

    m_idle_animation = m_entity_sprite->GetAnimationIdFromName("idle");
    m_turn_animation = m_entity_sprite->GetAnimationIdFromName("turn");
    m_attack_animation = m_entity_sprite->GetAnimationIdFromName("attack_turn");
    m_death_animation = m_entity_sprite->GetAnimationIdFromName("die");

    m_damage_system = system_context->GetSystem<game::DamageSystem>();
    DamageRecord* damage_record = m_damage_system->GetDamageRecord(entity_id);
    damage_record->release_entity_on_death = false;

    const DamageCallback destroyed_callback = [this](uint32_t damaged_entity_id, uint32_t who_did_damage, uint32_t weapon_identifier, int damage, DamageType type) {
        if(type == DamageType::DAMAGED)
            OnDamage(who_did_damage, damage);
        else if(type == DamageType::DESTROYED)
            m_states.TransitionTo(States::DEAD);
    };
    m_damage_system->SetDamageCallback(entity_id, DamageType::DT_ALL, destroyed_callback);

    m_target_system = system_context->GetSystem<TargetSystem>();

    const CacoStateMachine::StateTable state_table = {
        CacoStateMachine::MakeState(
            States::IDLE, &DemonMinionController::OnIdle, &DemonMinionController::Idle, this),
        CacoStateMachine::MakeState(
            States::ACTIVE, &DemonMinionController::OnActive, &DemonMinionController::Active, this),
        CacoStateMachine::MakeState(
            States::TURN_TO_PLAYER, &DemonMinionController::OnTurn, &DemonMinionController::TurnToPlayer, this),
        CacoStateMachine::MakeState(
            States::ACTION_FIRE_CIRCLE, &DemonMinionController::OnCircleAttack, &DemonMinionController::CircleAttack, this),
        CacoStateMachine::MakeState(
            States::ACTION_FIRE_HOMING, &DemonMinionController::OnFireHoming, &DemonMinionController::ActionFireHoming, this),
        CacoStateMachine::MakeState(
            States::ACTION_FIRE_LONG, &DemonMinionController::OnLongAttack, &DemonMinionController::ActionLongAttack, this),
        CacoStateMachine::MakeState(
            States::DEAD, &DemonMinionController::OnDead, &DemonMinionController::Dead, this),
    };
    m_states.SetStateTableAndState(state_table, States::IDLE);
}

DemonMinionController::~DemonMinionController()
{ }

void DemonMinionController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
    m_primary_weapon->UpdateWeaponState(update_context.timestamp);
    m_secondary_weapon->UpdateWeaponState(update_context.timestamp);
    m_tertiary_weapon->UpdateWeaponState(update_context.timestamp);

    m_circle_attack_cooldown = std::clamp(m_circle_attack_cooldown - update_context.delta_s, 0.0f, 10.0f);
    m_fire_homing_cooldown = std::clamp(m_fire_homing_cooldown - update_context.delta_s, 0.0f, 10.0f);
    m_long_attack_cooldown = std::clamp(m_long_attack_cooldown - update_context.delta_s, 0.0f, 10.0f);
}

void DemonMinionController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    debug_drawer->DrawCircle(world_position, tweak_values::activate_distance, mono::Color::GREEN);
    debug_drawer->DrawCircle(world_position, tweak_values::retreat_distance, mono::Color::MAGENTA);
    debug_drawer->DrawCircle(world_position, tweak_values::advance_distance, mono::Color::MAGENTA);
    debug_drawer->DrawCircle(world_position, tweak_values::circle_attack_distance, mono::Color::RED);
    debug_drawer->DrawCircle(world_position, tweak_values::homing_attack_distance, mono::Color::RED);
    debug_drawer->DrawCircle(world_position, tweak_values::long_attack_distance, mono::Color::RED);

    const char* state_string = StateToString(m_states.ActiveState());
    debug_drawer->DrawWorldText(state_string, world_position, mono::Color::MAGENTA);
}

const char* DemonMinionController::GetDebugCategory() const
{
    return "Demon Minion Boss";
}

void DemonMinionController::UpdateMovement(const mono::UpdateContext& update_context)
{
    if(!m_aquired_target || !m_aquired_target->IsValid())
        return;

    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector& target_world_position = m_aquired_target->Position();

    const float distance_to_target = math::DistanceBetween(world_position, target_world_position);
    const math::Vector position_diff_normalized = math::Normalized(world_position - target_world_position);

    if(distance_to_target < tweak_values::retreat_distance)
        m_entity_body->ApplyLocalImpulse(position_diff_normalized * 10.0f, math::ZeroVec);
    else if(distance_to_target > tweak_values::advance_distance)
        m_entity_body->ApplyLocalImpulse(-position_diff_normalized * 10.0f, math::ZeroVec);
}

void DemonMinionController::OnIdle()
{
    m_entity_sprite->SetAnimation(m_idle_animation);
    m_entity_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
}

void DemonMinionController::Idle(const mono::UpdateContext& update_context)
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);

    m_aquired_target = m_target_system->AquireTarget(TargetFaction::Player, world_position, tweak_values::activate_distance);
    if(!m_aquired_target->IsValid())
        return;

    UpdateMovement(update_context);

    const math::Vector& target_world_position = m_aquired_target->Position();
    const float distance_to_target = math::DistanceBetween(world_position, target_world_position);

    if(distance_to_target < tweak_values::circle_attack_distance && m_circle_attack_cooldown == 0.0f)
        TurnAndTransitionTo(States::ACTION_FIRE_CIRCLE);
    else if(distance_to_target < tweak_values::homing_attack_distance && m_fire_homing_cooldown == 0.0f)
        TurnAndTransitionTo(States::ACTION_FIRE_HOMING);
    else if(distance_to_target < tweak_values::long_attack_distance && m_long_attack_cooldown == 0.0f)
        TurnAndTransitionTo(States::ACTION_FIRE_LONG);
}

void DemonMinionController::OnActive()
{}

void DemonMinionController::Active(const mono::UpdateContext& update_context)
{}

void DemonMinionController::OnTurn()
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    const auto transition_to_attack = [this](uint32_t sprite_id) {
        m_states.TransitionTo(m_state_after_turn);
    };
    m_entity_sprite->SetAnimation(m_turn_animation, transition_to_attack);

    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector delta = position - m_aquired_target->Position();

    if(delta.x > 0.0f)
        m_entity_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
}

void DemonMinionController::TurnToPlayer(const mono::UpdateContext& update_context)
{ }

void DemonMinionController::OnCircleAttack()
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    m_ready_to_attack = false;

    const auto set_ready_to_attack = [this](uint32_t sprite_id) {
        m_ready_to_attack = true;
    };
    m_entity_sprite->SetAnimation(m_attack_animation, set_ready_to_attack);

    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector delta = position - m_aquired_target->Position();

    if(delta.x > 0.0f)
        m_entity_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
}

void DemonMinionController::CircleAttack(const mono::UpdateContext& update_context)
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    UpdateMovement(update_context);

    if(!m_ready_to_attack)
        return;

    const math::Vector& fire_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector& target_position = m_aquired_target->Position();

    const math::Vector delta_normalized = math::Normalized(target_position - fire_position);
    const WeaponState fire_result = m_secondary_weapon->Fire(fire_position, fire_position + (delta_normalized * 3.0f), update_context.timestamp);
    if(fire_result == WeaponState::OUT_OF_AMMO)
    {
        m_secondary_weapon->Reload(update_context.timestamp);
        m_circle_attack_cooldown = tweak_values::circle_attack_cooldown_s;
        m_states.TransitionTo(States::IDLE);
    }
}

void DemonMinionController::OnFireHoming()
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    m_ready_to_attack = false;

    const auto set_ready_to_attack = [this](uint32_t sprite_id) {
        m_ready_to_attack = true;
    };
    m_entity_sprite->SetAnimation(m_attack_animation, set_ready_to_attack);

    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector delta = position - m_aquired_target->Position();

    if(delta.x > 0.0f)
        m_entity_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
}

void DemonMinionController::ActionFireHoming(const mono::UpdateContext& update_context)
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    UpdateMovement(update_context);

    if(!m_ready_to_attack)
        return;

    const float x_diff = mono::Random(-0.2f, 0.2f);
    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector fire_position = position + math::Vector(x_diff, 0.0f);

    const WeaponState fire_result = m_primary_weapon->Fire(fire_position, m_aquired_target->Position(), update_context.timestamp);
    if(fire_result == WeaponState::OUT_OF_AMMO)
    {
        m_primary_weapon->Reload(update_context.timestamp);
        m_fire_homing_cooldown = tweak_values::homing_attack_cooldown_s;
        m_states.TransitionTo(States::IDLE);
    }
}

void DemonMinionController::OnLongAttack()
{
    m_ready_to_attack = false;

    const auto set_ready_to_attack = [this](uint32_t sprite_id) {
        m_ready_to_attack = true;
    };
    m_entity_sprite->SetAnimation(m_attack_animation, set_ready_to_attack);
}

void DemonMinionController::ActionLongAttack(const mono::UpdateContext& update_context)
{
    if(!m_aquired_target->IsValid())
    {
        m_states.TransitionTo(States::IDLE);
        return;
    }

    if(!m_ready_to_attack)
        return;

    const math::Vector& fire_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector& target_position = m_aquired_target->Position();

    const WeaponState fire_result = m_tertiary_weapon->Fire(fire_position, target_position, update_context.timestamp);
    if(fire_result == WeaponState::OUT_OF_AMMO)
    {
        m_tertiary_weapon->Reload(update_context.timestamp);
        m_long_attack_cooldown = tweak_values::long_attack_cooldown_s;
        m_states.TransitionTo(States::IDLE);
    }
}

void DemonMinionController::OnDead()
{
    m_entity_sprite->SetAnimation(m_death_animation);
    m_entity_sprite->ClearProperty(mono::SpriteProperty::SHADOW);

    m_death_sound->Play();
}

void DemonMinionController::Dead(const mono::UpdateContext& update_context)
{ }

const char* DemonMinionController::StateToString(States state) const
{
    switch(state)
    {
    case States::IDLE:
        return "Idle";
    case States::ACTIVE:
        return "Active";
    case States::TURN_TO_PLAYER:
        return "Turn_To_Player";
    case States::ACTION_FIRE_CIRCLE:
        return "Action_Fire_Circle";
    case States::ACTION_FIRE_HOMING:
        return "Action_Fire_Homing";
    case States::ACTION_FIRE_LONG:
        return "Action_Fire_Long";
    case States::DEAD:
        return "Dead";
    }

    return "Unknown";
}

void DemonMinionController::TurnAndTransitionTo(States state_after_turn)
{
    m_states.TransitionTo(States::TURN_TO_PLAYER);
    m_state_after_turn = state_after_turn;
}

void DemonMinionController::OnDamage(uint32_t who_did_damage, int damage)
{
    const DamageRecord* damage_record = m_damage_system->GetDamageRecord(m_entity_id);
    const float health_fraction = float(damage_record->health) / float(damage_record->full_health);
    if(health_fraction < 0.2f)
    {
        m_beast_mode = true;
    }

    const bool is_playing_already = m_damage_sound->IsPlaying();
    if(is_playing_already)
        return;

    const bool play_sound = mono::Chance(50);
    if(play_sound)
        m_damage_sound->Play();
}
