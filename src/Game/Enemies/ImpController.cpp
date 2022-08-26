
#include "ImpController.h"

#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "Math/MathFunctions.h"
#include "Math/EasingFunctions.h"
#include "Util/Random.h"
#include "IDebugDrawer.h"

#include "Player/PlayerInfo.h"
#include "SystemContext.h"
#include "Weapons/IWeapon.h"
#include "Weapons/WeaponSystem.h"


namespace tweak_values
{
    constexpr float activate_distance_to_player_threshold = 4.0f;
    constexpr float perpendicular_movement_distance_threshold = 2.0f;
    constexpr float move_speed = 0.5f;
    constexpr float degrees_per_second = 360.0f;
    //constexpr uint32_t attack_start_delay = 500;
    constexpr uint32_t attack_sequence_delay = 50;
    constexpr uint32_t n_attacks = 3;
}

using namespace game;

ImpController::ImpController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
{
    game::WeaponSystem* weapon_system = system_context->GetSystem<game::WeaponSystem>();
    m_weapon = weapon_system->CreateWeapon(game::FLAK_CANON, WeaponFaction::ENEMY, entity_id);

    m_transform_system = system_context->GetSystem<mono::TransformSystem>();

    mono::PhysicsSystem* physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    mono::IBody* body = physics_system->GetBody(entity_id);

    m_homing_behaviour.SetBody(body);
    m_homing_behaviour.SetForwardVelocity(tweak_values::move_speed);
    m_homing_behaviour.SetAngularVelocity(tweak_values::degrees_per_second);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    m_idle_anim_id = m_sprite->GetAnimationIdFromName("idle");
    m_run_anim_id = m_sprite->GetAnimationIdFromName("run");
    m_attack_anim_id = m_sprite->GetAnimationIdFromName("attack");

    const GoblinStateMachine::StateTable state_table = {
        GoblinStateMachine::MakeState(States::IDLE, &ImpController::ToIdle, &ImpController::Idle, this),
        GoblinStateMachine::MakeState(States::REPOSITION, &ImpController::ToReposition, &ImpController::Reposition, this),
        GoblinStateMachine::MakeState(States::PREPARE_ATTACK, &ImpController::ToPrepareAttack, &ImpController::PrepareAttack, this),
        GoblinStateMachine::MakeState(States::ATTACKING, &ImpController::ToAttacking, &ImpController::Attacking, this),
    };

    m_states.SetStateTableAndState(state_table, States::IDLE);
}

void ImpController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

void ImpController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);
    debug_drawer->DrawCircle(world_position, tweak_values::activate_distance_to_player_threshold, mono::Color::CYAN);
    debug_drawer->DrawCircle(world_position, tweak_values::perpendicular_movement_distance_threshold, mono::Color::GREEN);

    const math::Vector& target_position = m_homing_behaviour.GetTargetPosition();
    debug_drawer->DrawLine({ world_position, target_position }, 1.0f, mono::Color::BLUE);
    debug_drawer->DrawPoint(target_position, 10.0f, mono::Color::BLUE);

    debug_drawer->DrawLine({ world_position, m_attack_position }, 1.0f, mono::Color::RED);
    debug_drawer->DrawPoint(m_attack_position, 5.0f, mono::Color::RED);
}

const char* ImpController::GetDebugCategory() const
{
    return "Imp";
}

void ImpController::ToIdle()
{
    m_idle_timer = 0;
    m_sprite->SetShade(mono::Color::WHITE);
    m_sprite->SetAnimation(m_idle_anim_id);
}

void ImpController::Idle(const mono::UpdateContext& update_context)
{
    m_idle_timer += update_context.delta_ms;

    const math::Matrix& world_transform = m_transform_system->GetWorld(m_entity_id);
    const math::Vector& world_position = math::GetPosition(world_transform);

    const game::PlayerInfo* player_info = GetClosestActivePlayer(
        world_position, tweak_values::activate_distance_to_player_threshold);
    if(!player_info)
        return;

    m_attack_position = player_info->position;

    const bool is_left_of = (m_attack_position.x < world_position.x);
    if(is_left_of)
        m_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    else 
        m_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

    if(m_idle_timer < 1000)
        return;

    const bool transition_to_attack = mono::Chance(25);
    if(transition_to_attack)
        m_states.TransitionTo(States::PREPARE_ATTACK);
    else
        m_states.TransitionTo(States::REPOSITION);

    m_idle_timer = 0;
}

void ImpController::ToReposition()
{
    const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);
    const game::PlayerInfo* player_info = GetClosestActivePlayer(world_position);
    if(!player_info)
        return;

    const math::Vector delta = player_info->position - world_position;
    const math::Vector normalized_delta = math::Normalized(delta);

    math::Vector homing_target = world_position;

    const float distance_to_player = math::Length(delta);
    if(distance_to_player < tweak_values::perpendicular_movement_distance_threshold)
    {
        // Move sideways
        const float multiplier = mono::Chance(50) ? -1.0f : 1.0f;
        homing_target += math::Perpendicular(normalized_delta) * multiplier;
    }
    else
    {
        // Move towards player
        homing_target += normalized_delta;
    }

    m_sprite->SetAnimation(m_run_anim_id);
    m_homing_behaviour.SetTargetPosition(homing_target);
}

void ImpController::Reposition(const mono::UpdateContext& update_context)
{
    const game::HomingResult result = m_homing_behaviour.Run(update_context);
    if(result.distance_to_target < 0.1f)
    {
        const bool fire = mono::Chance(75);
        const States new_state = fire ? States::PREPARE_ATTACK : States::IDLE;
        m_states.TransitionTo(new_state);
    }
}

void ImpController::ToPrepareAttack()
{
    const auto transition_to_attacking = [this]() {
        m_states.TransitionTo(States::ATTACKING);
    };
    m_sprite->SetAnimation(m_attack_anim_id, transition_to_attacking);
}

void ImpController::PrepareAttack(const mono::UpdateContext& update_context)
{ }

void ImpController::ToAttacking()
{
    m_attack_timer = 0;
    m_n_attacks = 0;
}

void ImpController::Attacking(const mono::UpdateContext& update_context)
{
    if(m_n_attacks >= tweak_values::n_attacks || m_weapon->UpdateWeaponState(update_context.timestamp) == game::WeaponState::RELOADING)
        m_states.TransitionTo(States::IDLE);

    m_attack_timer += update_context.delta_ms;
    if(m_attack_timer > tweak_values::attack_sequence_delay)
    {
        const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);

        game::WeaponState fire_state = m_weapon->Fire(world_position, m_attack_position, update_context.timestamp);
        if(fire_state == game::WeaponState::FIRE)
            m_n_attacks++;
        else if(fire_state == game::WeaponState::OUT_OF_AMMO)
            m_weapon->Reload(update_context.timestamp);
    
        m_attack_timer = 0;
    }
}
