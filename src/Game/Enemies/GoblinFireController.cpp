
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

#include "Player/PlayerInfo.h"
#include "Factories.h"
#include "SystemContext.h"
#include "Weapons/IWeapon.h"
#include "Weapons/IWeaponFactory.h"


namespace tweak_values
{
    constexpr float activate_distance_to_player_threshold = 6.0f;
    constexpr float distance_to_player_threshold = 3.0f;
    constexpr float move_speed = 0.5f;
    constexpr float degrees_per_second = 360.0f;
    constexpr uint32_t attack_start_delay = 500;
    constexpr uint32_t attack_sequence_delay = 50;
    constexpr uint32_t n_attacks = 3;
}

using namespace game;

GoblinFireController::GoblinFireController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
{
    m_weapon = g_weapon_factory->CreateWeapon(game::CACO_PLASMA, WeaponFaction::ENEMY, entity_id);
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();

    mono::PhysicsSystem* physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    mono::IBody* body = physics_system->GetBody(entity_id);

    m_homing_behaviour.SetBody(body);
    m_homing_behaviour.SetForwardVelocity(tweak_values::move_speed);
    m_homing_behaviour.SetAngularVelocity(tweak_values::degrees_per_second);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    m_idle_anim_id = m_sprite->GetAnimationIdFromName("idle");
    m_run_anim_id = m_sprite->GetAnimationIdFromName("walk");

    const GoblinStateMachine::StateTable state_table = {
        GoblinStateMachine::MakeState(States::IDLE, &GoblinFireController::ToIdle, &GoblinFireController::Idle, this),
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

void GoblinFireController::ToIdle()
{
    m_idle_timer = 0;
    m_sprite->SetShade(mono::Color::WHITE);
    m_sprite->SetAnimation(m_idle_anim_id);
}

void GoblinFireController::Idle(const mono::UpdateContext& update_context)
{
    m_idle_timer += update_context.delta_ms;

    const math::Matrix& world_transform = m_transform_system->GetWorld(m_entity_id);
    const math::Vector& world_position = math::GetPosition(world_transform);

    bool is_visible = false;
    const game::PlayerInfo* player_info = GetClosestActivePlayer(world_position);
    if(player_info)
    {
        m_attack_position = player_info->position;
        const float distance = math::DistanceBetween(world_position, player_info->position);
        is_visible = (distance < tweak_values::activate_distance_to_player_threshold);
        //is_visible = math::PointInsideQuad(world_position, player_info->viewport);
    }

    const bool is_left_of = (m_attack_position.x < world_position.x);
    if(is_left_of)
        m_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    else 
        m_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

    const bool is_player_active = (player_info != nullptr);
    if(!is_player_active || !is_visible)
        return;

    if(m_idle_timer < 1000)
        return;

    const bool transition_to_attack = mono::Chance(25);
    if(transition_to_attack)
        m_states.TransitionTo(States::PREPARE_ATTACK);
    else
        m_states.TransitionTo(States::REPOSITION);

    m_idle_timer = 0;
}

void GoblinFireController::ToReposition()
{
    const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);
    const game::PlayerInfo* player_info = GetClosestActivePlayer(world_position);
    if(!player_info)
        return;

    const math::Vector delta = player_info->position - world_position;
    const float distance_to_player = math::Length(delta);

    math::Vector homing_target = world_position;

    // Move towards player
    const math::Vector halfway_delta = delta / 2.0f;

    if(distance_to_player < tweak_values::distance_to_player_threshold)
    {
        const float multiplier = mono::Chance(50) ? -1.0f : 1.0f;
        homing_target += math::Perpendicular(halfway_delta) * multiplier; // Move sideways
    }
    else
    {
        homing_target += halfway_delta;
    }

    m_sprite->SetAnimation(m_run_anim_id);
    m_homing_behaviour.SetTargetPosition(homing_target);
}

void GoblinFireController::Reposition(const mono::UpdateContext& update_context)
{
    const game::HomingResult result = m_homing_behaviour.Run(update_context);
    if(result.distance_to_target < 0.1f)
    {
        const bool fire = mono::Chance(75);
        const States new_state = fire ? States::PREPARE_ATTACK : States::IDLE;
        m_states.TransitionTo(new_state);
    }
}

void GoblinFireController::ToPrepareAttack()
{
    m_prepare_timer = 0;
    m_sprite->SetShade(mono::Color::RED);
}

void GoblinFireController::PrepareAttack(const mono::UpdateContext& update_context)
{
    m_prepare_timer += update_context.delta_ms;
    if(m_prepare_timer > tweak_values::attack_start_delay)
        m_states.TransitionTo(States::ATTACKING);
}

void GoblinFireController::ToAttacking()
{
    m_attack_timer = 0;
    m_n_attacks = 0;
}

void GoblinFireController::Attacking(const mono::UpdateContext& update_context)
{
    if(m_n_attacks >= tweak_values::n_attacks || m_weapon->UpdateWeaponState(update_context.timestamp) == game::WeaponState::RELOADING)
        m_states.TransitionTo(States::IDLE);

    m_attack_timer += update_context.delta_ms;
    if(m_attack_timer > tweak_values::attack_sequence_delay)
    {
        const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);

        const float angle = math::AngleBetweenPoints(m_attack_position, world_position) + math::PI_2();
        game::WeaponState fire_state = m_weapon->Fire(world_position, angle, update_context.timestamp);
        if(fire_state == game::WeaponState::FIRE)
            m_n_attacks++;
        else if(fire_state == game::WeaponState::OUT_OF_AMMO)
            m_weapon->Reload(update_context.timestamp);
    
        m_attack_timer = 0;
    }
}
