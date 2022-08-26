
#include "CacoDemonController.h"

#include "Player/PlayerInfo.h"
#include "DamageSystem.h"
#include "Factories.h"
#include "IDebugDrawer.h"
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
    constexpr float face_angle = math::ToRadians(5.0f);
    constexpr float retreat_distance = 3.0f;
    constexpr float advance_distance = 4.5f;
    constexpr float attack_distance = 4.0f;
    constexpr float shockwave_distance = 1.5f;

    constexpr float shockwave_cooldown_s = 3.0f;
    constexpr float homing_attack_cooldown_s = 3.0f;
    constexpr float beam_attack_cooldown_s = 3.0f;
}

using namespace game;

CacodemonController::CacodemonController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
    , m_shockwave_cooldown(0.0f)
    , m_fire_homing_cooldown(0.0f)
    , m_fire_beam_cooldown(0.0f)
{
    game::WeaponSystem* weapon_system = system_context->GetSystem<game::WeaponSystem>();
    m_primary_weapon = weapon_system->CreateWeapon(game::CACO_PLASMA, WeaponFaction::ENEMY, entity_id);
    m_secondary_weapon = weapon_system->CreateWeapon(game::CACO_PLASMA_HOMING, WeaponFaction::ENEMY, entity_id);

    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_entity_body = m_physics_system->GetBody(entity_id);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_entity_sprite = sprite_system->GetSprite(entity_id);

    m_idle_animation = m_entity_sprite->GetAnimationIdFromName("idle");
    m_turn_animation = m_entity_sprite->GetAnimationIdFromName("turn");
    m_attack_animation = m_entity_sprite->GetAnimationIdFromName("attack_turn");
    m_death_animation = m_entity_sprite->GetAnimationIdFromName("dead");

    mono::ParticleSystem* particle_system = system_context->GetSystem<mono::ParticleSystem>();
    mono::IEntityManager* entity_manager = system_context->GetSystem<mono::IEntityManager>();

    m_shockwave_effect = std::make_unique<ShockwaveEffect>(m_transform_system, particle_system, entity_manager);

    game::DamageSystem* damage_system = system_context->GetSystem<game::DamageSystem>();
    damage_system->PreventReleaseOnDeath(entity_id, true);

    const DamageCallback destroyed_callback = [this](uint32_t id, int damage, uint32_t who_did_damage, DamageType type) {
        m_states.TransitionTo(States::DEAD);
    };
    damage_system->SetDamageCallback(entity_id, DamageType::DESTROYED, destroyed_callback);

    const CacoStateMachine::StateTable state_table = {
        CacoStateMachine::MakeState(
            States::IDLE, &CacodemonController::OnIdle, &CacodemonController::Idle, this),
        CacoStateMachine::MakeState(
            States::ACTIVE, &CacodemonController::OnActive, &CacodemonController::Active, this),
        CacoStateMachine::MakeState(
            States::TURN_TO_PLAYER, &CacodemonController::OnTurn, &CacodemonController::TurnToPlayer, this),
        CacoStateMachine::MakeState(
            States::ACTION_SHOCKWAVE, &CacodemonController::OnAction, &CacodemonController::ActionShockwave, this),
        CacoStateMachine::MakeState(
            States::ACTION_FIRE_HOMING, &CacodemonController::OnFireHoming, &CacodemonController::ActionFireHoming, this),
        CacoStateMachine::MakeState(
            States::ACTION_FIRE_BEAM, &CacodemonController::OnAction, &CacodemonController::ActionFireBeam, this),
        CacoStateMachine::MakeState(
            States::DEAD, &CacodemonController::OnDead, &CacodemonController::Dead, this),
    };
    m_states.SetStateTableAndState(state_table, States::IDLE);
}

CacodemonController::~CacodemonController()
{ }

void CacodemonController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
    m_primary_weapon->UpdateWeaponState(update_context.timestamp);
    m_secondary_weapon->UpdateWeaponState(update_context.timestamp);

    m_shockwave_cooldown = std::clamp(m_shockwave_cooldown - update_context.delta_s, 0.0f, 10.0f);
    m_fire_homing_cooldown = std::clamp(m_fire_homing_cooldown - update_context.delta_s, 0.0f, 10.0f);
    m_fire_beam_cooldown = std::clamp(m_fire_beam_cooldown - update_context.delta_s, 0.0f, 10.0f);
}

void CacodemonController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    debug_drawer->DrawCircle(world_position, tweak_values::retreat_distance, mono::Color::MAGENTA);
    debug_drawer->DrawCircle(world_position, tweak_values::advance_distance, mono::Color::MAGENTA);
    debug_drawer->DrawCircle(world_position, tweak_values::attack_distance, mono::Color::RED);
    debug_drawer->DrawCircle(world_position, tweak_values::shockwave_distance, mono::Color::RED);
}

const char* CacodemonController::GetDebugCategory() const
{
    return "Cacodemon";
}

void CacodemonController::OnIdle()
{
    m_entity_sprite->SetAnimation(m_idle_animation);
    m_entity_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
}

void CacodemonController::Idle(const mono::UpdateContext& update_context)
{
    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
    m_target_player = game::GetClosestActivePlayer(position);
    if(!m_target_player)
        return;

    /*
    const float current_rotation = math::GetZRotation(world_transform);
    const float angle_diff = current_rotation - target_angle - math::PI_2() + math::PI();
    const float angle_diff_abs = std::fabs(math::NormalizeAngle(angle_diff) - math::PI() * 2.0f);

    if(angle_diff_abs >= tweak_values::face_angle)
    {
        const float direction = angle_diff > 0.0f ? -1.0f : 1.0f;
        const float add = tweak_values::radians_per_second * update_context.delta_s * direction;
        m_entity_body->SetAngle(current_rotation + add);
    }
    */


    const math::Vector position_diff = position - m_target_player->position;
    const math::Vector position_diff_normalized = math::Normalized(position_diff);
    const float distance_to_player = math::Length(position_diff);
    if(distance_to_player < tweak_values::retreat_distance)
        m_entity_body->ApplyLocalImpulse(position_diff_normalized * 10.0f, math::ZeroVec);
    else if(distance_to_player > tweak_values::advance_distance)
        m_entity_body->ApplyLocalImpulse(-position_diff_normalized * 10.0f, math::ZeroVec);

    if(distance_to_player < tweak_values::attack_distance && m_fire_homing_cooldown == 0.0f)
        m_states.TransitionTo(States::TURN_TO_PLAYER);
    else if(distance_to_player < tweak_values::shockwave_distance && m_shockwave_cooldown == 0.0f)
        m_states.TransitionTo(States::ACTION_SHOCKWAVE);
}

void CacodemonController::OnActive()
{}

void CacodemonController::Active(const mono::UpdateContext& update_context)
{}

void CacodemonController::OnTurn()
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

void CacodemonController::TurnToPlayer(const mono::UpdateContext& update_context)
{ }

void CacodemonController::OnAction()
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

void CacodemonController::ActionShockwave(const mono::UpdateContext& update_context)
{
    if(!m_ready_to_attack)
        return;

    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
    game::ShockwaveAtForTypes(m_physics_system, position, 100.0f, PLAYER | PLAYER_BULLET);

    m_shockwave_effect->EmittAt(position);

    m_shockwave_cooldown = tweak_values::shockwave_cooldown_s;
    m_states.TransitionTo(States::IDLE);
}

void CacodemonController::OnFireHoming()
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

void CacodemonController::ActionFireHoming(const mono::UpdateContext& update_context)
{
    if(!m_ready_to_attack)
        return;

    //const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
    //const math::Vector delta = position - m_target_player->position;

    const float x_diff = mono::Random(-0.2f, 0.2f);

    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector fire_position = position + math::Vector(x_diff, 0.0f); //-0.35f);
    const math::Vector target_position = m_target_player->position; //position + math::Vector(0.0f, 2.0f);

    const WeaponState fire_result = m_secondary_weapon->Fire(fire_position, target_position, update_context.timestamp);
    if(fire_result == WeaponState::OUT_OF_AMMO)
    {
        m_secondary_weapon->Reload(update_context.timestamp);
        m_fire_homing_cooldown = tweak_values::homing_attack_cooldown_s;
        m_states.TransitionTo(States::IDLE);
    }
}

void CacodemonController::ActionFireBeam(const mono::UpdateContext& update_context)
{
    if(!m_ready_to_attack)
        return;

    const math::Vector& fire_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector& target_position = m_target_player->position;

    const WeaponState fire_result = m_primary_weapon->Fire(fire_position, target_position, update_context.timestamp);
    if(fire_result == WeaponState::OUT_OF_AMMO)
    {
        m_primary_weapon->Reload(update_context.timestamp);
        m_fire_beam_cooldown = tweak_values::beam_attack_cooldown_s;
        m_states.TransitionTo(States::IDLE);
    }
}

void CacodemonController::OnDead()
{
    m_entity_sprite->SetAnimation(m_death_animation);
    m_entity_sprite->ClearProperty(mono::SpriteProperty::SHADOW);
}

void CacodemonController::Dead(const mono::UpdateContext& update_context)
{ }
