
#include "CacoDemonController.h"

#include "Player/PlayerInfo.h"
#include "DamageSystem.h"
#include "Factories.h"
#include "Weapons/IWeapon.h"
#include "Weapons/IWeaponFactory.h"
#include "Shockwave.h"
#include "CollisionConfiguration.h"

#include "Math/MathFunctions.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"

#include <cmath>

namespace tweak_values
{
    constexpr float radians_per_second = math::ToRadians(90.0f);
    constexpr float face_angle = math::ToRadians(5.0f);
    constexpr float retreat_distance = 3.5f;
    constexpr float advance_distance = 5.5f;
}

using namespace game;

CacodemonController::CacodemonController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
{
    m_primary_weapon = g_weapon_factory->CreateWeapon(game::CACO_PLASMA, WeaponFaction::ENEMY, entity_id);
    m_secondary_weapon = g_weapon_factory->CreateWeapon(game::PLASMA_GUN_HOMING, WeaponFaction::ENEMY, entity_id);

    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_entity_body = m_physics_system->GetBody(entity_id);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    sprite_system->SetSpriteLayer(entity_id, -1);

    m_entity_sprite = sprite_system->GetSprite(entity_id);

    m_idle_animation = m_entity_sprite->GetAnimationIdFromName("idle");
    m_attack_animation = m_entity_sprite->GetAnimationIdFromName("attack");
    m_death_animation = m_entity_sprite->GetAnimationIdFromName("dead");


    game::DamageSystem* damage_system = system_context->GetSystem<game::DamageSystem>();
    damage_system->PreventReleaseOnDeath(entity_id, true);

    const DamageCallback destroyed_callback = [this](uint32_t id, int damage, uint32_t who_did_damage, DamageType type) {
        m_states.TransitionTo(States::DEAD);
    };
    damage_system->SetDamageCallback(entity_id, DamageType::DESTROYED, destroyed_callback);

    const StateMachine::StateTable state_table = {
        StateMachine::MakeState(
            States::IDLE, &CacodemonController::OnIdle, &CacodemonController::Idle, this),
        StateMachine::MakeState(
            States::ACTIVE, &CacodemonController::OnActive, &CacodemonController::Active, this),
        StateMachine::MakeState(
            States::ACTION_SHOCKWAVE, &CacodemonController::OnAction, &CacodemonController::ActionShockwave, this),
        StateMachine::MakeState(
            States::ACTION_FIRE_HOMING, &CacodemonController::OnAction, &CacodemonController::ActionFireHoming, this),
        StateMachine::MakeState(
            States::ACTION_FIRE_BEAM, &CacodemonController::OnAction, &CacodemonController::ActionFireBeam, this),
        StateMachine::MakeState(
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
}

void CacodemonController::OnIdle()
{
    m_entity_sprite->SetAnimation(m_idle_animation);
}

void CacodemonController::Idle(const mono::UpdateContext& update_context)
{
    const math::Matrix world_transform = m_transform_system->GetWorld(m_entity_id);
    const math::Vector position = math::GetPosition(world_transform);

    const PlayerInfo* closest_player = game::GetClosestActivePlayer(position);

    bool is_visible = false;
    float target_angle = math::PI_2();

    if(closest_player)
    {
        is_visible = math::PointInsideQuad(position, closest_player->viewport);
        if(is_visible)
            target_angle = math::AngleBetweenPoints(closest_player->position, position);
    }

    const float current_rotation = math::GetZRotation(world_transform);
    const float angle_diff = current_rotation - target_angle - math::PI_2() + math::PI();
    const float angle_diff_abs = std::fabs(math::NormalizeAngle(angle_diff) - math::PI() * 2.0f);

    if(angle_diff_abs >= tweak_values::face_angle)
    {
        const float direction = angle_diff > 0.0f ? -1.0f : 1.0f;
        const float add = tweak_values::radians_per_second * update_context.delta_s * direction;
        m_entity_body->SetAngle(current_rotation + add);
    }

    if(!is_visible || !closest_player)
        return;

    const math::Vector position_diff = position - closest_player->position;
    const math::Vector position_diff_normalized = math::Normalized(position_diff);
    const float distance_to_player = math::Length(position_diff);
    if(distance_to_player < tweak_values::retreat_distance)
        m_entity_body->ApplyLocalImpulse(position_diff_normalized * 200.0f, math::ZeroVec);
    else if(distance_to_player > tweak_values::advance_distance)
        m_entity_body->ApplyLocalImpulse(-position_diff_normalized * 200.0f, math::ZeroVec);

    if(angle_diff_abs <= tweak_values::face_angle)
        m_states.TransitionTo(States::ACTION_FIRE_HOMING);
}

void CacodemonController::OnActive()
{}

void CacodemonController::Active(const mono::UpdateContext& update_context)
{}

void CacodemonController::OnAction()
{
    m_ready_to_attack = false;

    const auto set_ready_to_attack = [this] {
        m_ready_to_attack = true;
    };
    m_entity_sprite->SetAnimation(m_attack_animation, set_ready_to_attack);
}

void CacodemonController::ActionShockwave(const mono::UpdateContext& update_context)
{
    if(!m_ready_to_attack)
        return;

    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
    game::ShockwaveAtForTypes(m_physics_system, position, 10.0f, PLAYER | PLAYER_BULLET);

    m_states.TransitionTo(States::ACTIVE);
}

void CacodemonController::ActionFireHoming(const mono::UpdateContext& update_context)
{
    if(!m_ready_to_attack)
        return;

    const math::Matrix world_transform = m_transform_system->GetWorld(m_entity_id);
    const math::Vector position = math::GetPosition(world_transform);
    const float rotation = math::GetZRotation(world_transform) + math::PI();

    const math::Vector offset_vector = math::VectorFromAngle(rotation) * 0.5f;

    const WeaponState fire_result = m_secondary_weapon->Fire(position + offset_vector, rotation, update_context.timestamp);
    if(fire_result == WeaponState::OUT_OF_AMMO)
    {
        m_secondary_weapon->Reload(update_context.timestamp);
        m_states.TransitionTo(States::IDLE);
    }

    //m_states.TransitionTo(States::ACTIVE);
}

void CacodemonController::ActionFireBeam(const mono::UpdateContext& update_context)
{
    if(!m_ready_to_attack)
        return;

    const math::Matrix world_transform = m_transform_system->GetWorld(m_entity_id);
    const math::Vector position = math::GetPosition(world_transform);
    const float rotation = math::GetZRotation(world_transform) + math::PI();

    const math::Vector offset_vector = math::VectorFromAngle(rotation) * 0.5f;

    const WeaponState fire_result = m_primary_weapon->Fire(position + offset_vector, rotation, update_context.timestamp);
    if(fire_result == WeaponState::OUT_OF_AMMO)
    {
        m_primary_weapon->Reload(update_context.timestamp);
        m_states.TransitionTo(States::IDLE);
    }

    m_states.TransitionTo(States::ACTIVE);
}

void CacodemonController::OnDead()
{
    m_entity_sprite->SetAnimation(m_death_animation);
    m_entity_sprite->ClearProperty(mono::SpriteProperty::SHADOW);
}

void CacodemonController::Dead(const mono::UpdateContext& update_context)
{ }
