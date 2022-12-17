
#include "DemonBossController.h"

#include "Player/PlayerInfo.h"
#include "DamageSystem/DamageSystem.h"
#include "Factories.h"
#include "Debug/IDebugDrawer.h"
#include "Weapons/IWeapon.h"
#include "Weapons/WeaponSystem.h"
#include "Shockwave.h"
#include "CollisionConfiguration.h"
#include "Effects/ShockwaveEffect.h"

#include "EntitySystem/IEntityManager.h"
#include "Math/MathFunctions.h"
#include "Particle/ParticleSystem.h"
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
    constexpr float attack_distance = 4.0f;
    constexpr float shockwave_distance = 1.5f;

    constexpr float shockwave_cooldown_s = 3.0f;
    constexpr float homing_attack_cooldown_s = 3.0f;
    constexpr float beam_attack_cooldown_s = 3.0f;
}

using namespace game;

DemonBossController::DemonBossController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
    , m_shockwave_cooldown(0.0f)
    , m_fire_homing_cooldown(0.0f)
    , m_fire_beam_cooldown(0.0f)
    , m_beast_mode(false)
{
    game::WeaponSystem* weapon_system = system_context->GetSystem<game::WeaponSystem>();
    m_primary_weapon = weapon_system->CreatePrimaryWeapon(entity_id, WeaponFaction::ENEMY);
    m_secondary_weapon = weapon_system->CreateSecondaryWeapon(entity_id, WeaponFaction::ENEMY);

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

    mono::ParticleSystem* particle_system = system_context->GetSystem<mono::ParticleSystem>();
    mono::IEntityManager* entity_manager = system_context->GetSystem<mono::IEntityManager>();

    m_shockwave_effect = std::make_unique<ShockwaveEffect>(m_transform_system, particle_system, entity_manager);

    m_damage_system = system_context->GetSystem<game::DamageSystem>();
    m_damage_system->PreventReleaseOnDeath(entity_id, true);

    const DamageCallback destroyed_callback = [this](uint32_t id, int damage, uint32_t who_did_damage, DamageType type) {
        if(type == DamageType::DAMAGED)
            OnDamage(who_did_damage, damage);
        else if(type == DamageType::DESTROYED)
            m_states.TransitionTo(States::DEAD);
    };
    m_damage_system->SetDamageCallback(entity_id, DamageType::DT_ALL, destroyed_callback);

    const CacoStateMachine::StateTable state_table = {
        CacoStateMachine::MakeState(
            States::IDLE, &DemonBossController::OnIdle, &DemonBossController::Idle, this),
        CacoStateMachine::MakeState(
            States::ACTIVE, &DemonBossController::OnActive, &DemonBossController::Active, this),
        CacoStateMachine::MakeState(
            States::TURN_TO_PLAYER, &DemonBossController::OnTurn, &DemonBossController::TurnToPlayer, this),
        CacoStateMachine::MakeState(
            States::ACTION_SHOCKWAVE, &DemonBossController::OnAction, &DemonBossController::ActionShockwave, this),
        CacoStateMachine::MakeState(
            States::ACTION_FIRE_HOMING, &DemonBossController::OnFireHoming, &DemonBossController::ActionFireHoming, this),
        CacoStateMachine::MakeState(
            States::ACTION_FIRE_BEAM, &DemonBossController::OnAction, &DemonBossController::ActionFireBeam, this),
        CacoStateMachine::MakeState(
            States::DEAD, &DemonBossController::OnDead, &DemonBossController::Dead, this),
    };
    m_states.SetStateTableAndState(state_table, States::IDLE);
}

DemonBossController::~DemonBossController()
{ }

void DemonBossController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
    m_primary_weapon->UpdateWeaponState(update_context.timestamp);
    m_secondary_weapon->UpdateWeaponState(update_context.timestamp);

    m_shockwave_cooldown = std::clamp(m_shockwave_cooldown - update_context.delta_s, 0.0f, 10.0f);
    m_fire_homing_cooldown = std::clamp(m_fire_homing_cooldown - update_context.delta_s, 0.0f, 10.0f);
    m_fire_beam_cooldown = std::clamp(m_fire_beam_cooldown - update_context.delta_s, 0.0f, 10.0f);
}

void DemonBossController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    debug_drawer->DrawCircle(world_position, tweak_values::activate_distance, mono::Color::GREEN);
    debug_drawer->DrawCircle(world_position, tweak_values::retreat_distance, mono::Color::MAGENTA);
    debug_drawer->DrawCircle(world_position, tweak_values::advance_distance, mono::Color::MAGENTA);
    debug_drawer->DrawCircle(world_position, tweak_values::attack_distance, mono::Color::RED);
    debug_drawer->DrawCircle(world_position, tweak_values::shockwave_distance, mono::Color::RED);

    const char* state_string = StateToString(m_states.ActiveState());
    debug_drawer->DrawWorldText(state_string, world_position, mono::Color::MAGENTA);
}

const char* DemonBossController::GetDebugCategory() const
{
    return "Green Boss Demon";
}

void DemonBossController::OnIdle()
{
    m_entity_sprite->SetAnimation(m_idle_animation);
    m_entity_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
}

void DemonBossController::Idle(const mono::UpdateContext& update_context)
{
    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
    m_target_player = game::GetClosestActivePlayer(position);
    if(!m_target_player)
        return;

    const float distance_to_player = math::DistanceBetween(position, m_target_player->position);
    if(distance_to_player > tweak_values::activate_distance)
        return;

    const math::Vector position_diff_normalized = math::Normalized(position - m_target_player->position);

    if(distance_to_player < tweak_values::retreat_distance)
        m_entity_body->ApplyLocalImpulse(position_diff_normalized * 10.0f, math::ZeroVec);
    else if(distance_to_player > tweak_values::advance_distance)
        m_entity_body->ApplyLocalImpulse(-position_diff_normalized * 10.0f, math::ZeroVec);

    if(distance_to_player < tweak_values::attack_distance && m_fire_homing_cooldown == 0.0f)
        m_states.TransitionTo(States::TURN_TO_PLAYER);
    else if(distance_to_player < tweak_values::shockwave_distance && m_shockwave_cooldown == 0.0f)
        m_states.TransitionTo(States::ACTION_SHOCKWAVE);
}

void DemonBossController::OnActive()
{}

void DemonBossController::Active(const mono::UpdateContext& update_context)
{}

void DemonBossController::OnTurn()
{
    const auto transition_to_attack = [this]() {
        m_states.TransitionTo(States::ACTION_FIRE_HOMING);
    };
    m_entity_sprite->SetAnimation(m_turn_animation, transition_to_attack);

    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector delta = position - m_target_player->position;

    if(delta.x > 0.0f)
        m_entity_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
}

void DemonBossController::TurnToPlayer(const mono::UpdateContext& update_context)
{ }

void DemonBossController::OnAction()
{
    m_ready_to_attack = false;

    const auto set_ready_to_attack = [this] {
        m_ready_to_attack = true;
    };
    m_entity_sprite->SetAnimation(m_attack_animation, set_ready_to_attack);

    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector delta = position - m_target_player->position;

    if(delta.x > 0.0f)
        m_entity_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
}

void DemonBossController::ActionShockwave(const mono::UpdateContext& update_context)
{
    if(!m_ready_to_attack)
        return;

    const math::Vector& fire_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector& target_position = m_target_player->position;

    const WeaponState fire_result = m_secondary_weapon->Fire(fire_position, target_position, update_context.timestamp);
    if(fire_result == WeaponState::OUT_OF_AMMO)
    {
        m_secondary_weapon->Reload(update_context.timestamp);
        m_fire_beam_cooldown = tweak_values::beam_attack_cooldown_s;
        m_states.TransitionTo(States::IDLE);
    }

/*
    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
    game::ShockwaveAtForTypes(m_physics_system, position, 100.0f, PLAYER | PLAYER_BULLET);

    m_shockwave_effect->EmittAt(position);

    m_shockwave_cooldown = tweak_values::shockwave_cooldown_s;
    m_states.TransitionTo(States::IDLE);
    */
}

void DemonBossController::OnFireHoming()
{
    m_ready_to_attack = false;

    const auto set_ready_to_attack = [this] {
        m_ready_to_attack = true;
    };
    m_entity_sprite->SetAnimation(m_attack_animation, set_ready_to_attack);

    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector delta = position - m_target_player->position;

    if(delta.x > 0.0f)
        m_entity_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
}

void DemonBossController::ActionFireHoming(const mono::UpdateContext& update_context)
{
    if(!m_ready_to_attack)
        return;

    const float x_diff = mono::Random(-0.2f, 0.2f);
    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector fire_position = position + math::Vector(x_diff, 0.0f);

    const WeaponState fire_result = m_primary_weapon->Fire(fire_position, m_target_player->position, update_context.timestamp);
    if(fire_result == WeaponState::OUT_OF_AMMO)
    {
        m_primary_weapon->Reload(update_context.timestamp);
        m_fire_homing_cooldown = tweak_values::homing_attack_cooldown_s;
        m_states.TransitionTo(States::IDLE);
    }
}

void DemonBossController::ActionFireBeam(const mono::UpdateContext& update_context)
{
    if(!m_ready_to_attack)
        return;

    const math::Vector& fire_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector& target_position = m_target_player->position;

    const WeaponState fire_result = m_secondary_weapon->Fire(fire_position, target_position, update_context.timestamp);
    if(fire_result == WeaponState::OUT_OF_AMMO)
    {
        m_secondary_weapon->Reload(update_context.timestamp);
        m_fire_beam_cooldown = tweak_values::beam_attack_cooldown_s;
        m_states.TransitionTo(States::IDLE);
    }
}

void DemonBossController::OnDead()
{
    m_entity_sprite->SetAnimation(m_death_animation);
    m_entity_sprite->ClearProperty(mono::SpriteProperty::SHADOW);

    m_death_sound->Play();
}

void DemonBossController::Dead(const mono::UpdateContext& update_context)
{ }

const char* DemonBossController::StateToString(States state) const
{
    switch(state)
    {
    case States::IDLE:
        return "Idle";
    case States::ACTIVE:
        return "Active";
    case States::TURN_TO_PLAYER:
        return "Turn_To_Player";
    case States::ACTION_SHOCKWAVE:
        return "Action_Shockwave";
    case States::ACTION_FIRE_HOMING:
        return "Action_Fire_Homing";
    case States::ACTION_FIRE_BEAM:
        return "Action_Fire_Beam";
    case States::DEAD:
        return "Dead";
    }

    return "Unknown";
}

void DemonBossController::OnDamage(uint32_t who_did_damage, int damage)
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
    if(play_sound || true)
        m_damage_sound->Play();
}
