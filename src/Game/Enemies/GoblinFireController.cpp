
#include "GoblinFireController.h"
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
    constexpr float move_speed = 1.0f;
    constexpr uint32_t attack_start_delay = 500;
    constexpr uint32_t attack_sequence_delay = 50;
    constexpr uint32_t n_attacks = 3;

    constexpr math::EaseFunction ease_function = math::LinearTween;
}

using namespace game;

GoblinFireController::GoblinFireController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
{
    m_weapon = g_weapon_factory->CreateWeapon(game::CACO_PLASMA, WeaponFaction::ENEMY, entity_id);
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    m_idle_anim_id = m_sprite->GetAnimationIdFromName("idle");
    m_run_anim_id = m_sprite->GetAnimationIdFromName("walk");

    const GoblinStateMachine::StateTable state_table = {
        GoblinStateMachine::MakeState(GoblinStates::IDLE, &GoblinFireController::ToIdle, &GoblinFireController::Idle, this),
        GoblinStateMachine::MakeState(GoblinStates::REPOSITION, &GoblinFireController::ToReposition, &GoblinFireController::Reposition, this),
        GoblinStateMachine::MakeState(GoblinStates::PREPARE_ATTACK, &GoblinFireController::ToPrepareAttack, &GoblinFireController::PrepareAttack, this),
        GoblinStateMachine::MakeState(GoblinStates::ATTACKING, &GoblinFireController::ToAttacking, &GoblinFireController::Attacking, this),
    };

    m_states.SetStateTableAndState(state_table, GoblinStates::IDLE);
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
        m_states.TransitionTo(GoblinStates::PREPARE_ATTACK);
    else
        m_states.TransitionTo(GoblinStates::REPOSITION);

    m_idle_timer = 0;
}

void GoblinFireController::ToReposition()
{
    const math::Matrix& world_transform = m_transform_system->GetWorld(m_entity_id);
    const math::Vector& world_position = math::GetPosition(world_transform);

    m_start_position = world_position;

    const game::PlayerInfo* player_info = GetClosestActivePlayer(world_position);
    if(!player_info)
        return;

    const math::Vector delta = player_info->position - world_position;
    
    // Move towards player
    m_move_delta = delta / 2.0f;
    
    const float distance_to_player = math::Length(delta);

    if(distance_to_player < tweak_values::distance_to_player_threshold)
    {
        const float multiplier = mono::Chance(50) ? -1.0f : 1.0f;
        m_move_delta = math::Perpendicular(m_move_delta) * multiplier; // Move sideways
    }

    m_move_counter = 0.0f;
    m_sprite->SetAnimation(m_run_anim_id);
}

void GoblinFireController::Reposition(const mono::UpdateContext& update_context)
{
    const float duration = math::Length(m_move_delta) / tweak_values::move_speed;

    math::Vector new_position;
    new_position.x = tweak_values::ease_function(m_move_counter, duration, m_start_position.x, m_move_delta.x);
    new_position.y = tweak_values::ease_function(m_move_counter, duration, m_start_position.y, m_move_delta.y);

    math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);
    math::Position(transform, new_position);
    m_transform_system->SetTransformState(m_entity_id, mono::TransformState::CLIENT);

    m_move_counter += update_context.delta_s;

    if(m_move_counter >= duration)
    {
        const bool fire = mono::Chance(75);
        const GoblinStates new_state = fire ? GoblinStates::PREPARE_ATTACK : GoblinStates::IDLE;
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
        m_states.TransitionTo(GoblinStates::ATTACKING);
}

void GoblinFireController::ToAttacking()
{
    m_attack_timer = 0;
    m_n_attacks = 0;
}

void GoblinFireController::Attacking(const mono::UpdateContext& update_context)
{
    if(m_n_attacks >= tweak_values::n_attacks || m_weapon->UpdateWeaponState(update_context.timestamp) == game::WeaponState::RELOADING)
        m_states.TransitionTo(GoblinStates::IDLE);

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
