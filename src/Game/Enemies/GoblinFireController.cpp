
#include "GoblinFireController.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "Math/MathFunctions.h"
#include "Util/Random.h"

#include "AIKnowledge.h"
#include "Factories.h"
#include "SystemContext.h"
#include "Weapons/IWeapon.h"
#include "Weapons/IWeaponFactory.h"

using namespace game;

GoblinFireController::GoblinFireController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
{
    m_weapon = g_weapon_factory->CreateWeapon(WeaponType::CACOPLASMA, WeaponFaction::ENEMY, entity_id);
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    const GoblinStateMachine::StateTable state_table = {
        GoblinStateMachine::MakeState(GoblinStates::IDLE, &GoblinFireController::ToIdle, &GoblinFireController::Idle, this),
        GoblinStateMachine::MakeState(GoblinStates::MOVING, &GoblinFireController::ToMoving, &GoblinFireController::Moving, this),
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
}

void GoblinFireController::Idle(const mono::UpdateContext& update_context)
{
    m_idle_timer += update_context.delta_ms;

    const math::Matrix& world_transform = m_transform_system->GetWorld(m_entity_id);
    const math::Vector& world_position = math::GetPosition(world_transform);

    const bool is_player_active = g_player_one.player_state == game::PlayerState::ALIVE;
    const bool is_visible = math::PointInsideQuad(world_position, g_camera_viewport);
    if(is_player_active && is_visible && m_idle_timer > 1000)
    {
        const float percentage = mono::Random();
        if(percentage <= 0.2f)
            m_states.TransitionTo(GoblinStates::PREPARE_ATTACK);
        
        m_idle_timer = 0;
    }
}

void GoblinFireController::ToMoving()
{
    /*
    const math::Matrix& world_transform = m_transform_system->GetWorld(m_entity_id);
    m_current_position = math::GetPosition(world_transform);

    constexpr float move_radius = tweek_values::move_radius;
    const float x = mono::Random(-move_radius, move_radius);
    const float y = mono::Random(-move_radius, move_radius);

    m_move_delta = (m_start_position + math::Vector(x, y)) - m_current_position;
    m_move_counter = 0.0f;

    const mono::HorizontalDirection new_horizontal_direction =
        m_move_delta.x < 0.0f ? mono::HorizontalDirection::LEFT : mono::HorizontalDirection::RIGHT;

    mono::ISprite* sprite = m_sprite_system->GetSprite(m_entity_id);
    sprite->SetHorizontalDirection(new_horizontal_direction);
*/
}

void GoblinFireController::Moving(const mono::UpdateContext& update_context)
{
    /*
    const float duration = math::Length(m_move_delta) / tweek_values::move_speed;

    math::Vector new_position;
    new_position.x = tweek_values::ease_function(m_move_counter, duration, m_current_position.x, m_move_delta.x);
    new_position.y = tweek_values::ease_function(m_move_counter, duration, m_current_position.y, m_move_delta.y);

    math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);
    math::Position(transform, new_position);

    m_move_counter += float(update_context.delta_ms) / 1000.0f;

    if(m_move_counter >= duration)
        m_states.TransitionTo(GoblinStates::IDLE);
    */
}

void GoblinFireController::ToPrepareAttack()
{
    m_prepare_timer = 0;
    m_sprite->SetShade(mono::Color::RED);
}

void GoblinFireController::PrepareAttack(const mono::UpdateContext& update_context)
{
    m_prepare_timer += update_context.delta_ms;
    if(m_prepare_timer > 1000)
        m_states.TransitionTo(GoblinStates::ATTACKING);
}

void GoblinFireController::ToAttacking()
{
    m_attack_timer = 0;
    m_n_attacks = 0;
}

void GoblinFireController::Attacking(const mono::UpdateContext& update_context)
{
    if(g_player_one.player_state != game::PlayerState::ALIVE || m_n_attacks >= 3)
        m_states.TransitionTo(GoblinStates::IDLE);

    m_attack_timer += update_context.delta_ms;
    if(m_attack_timer > 50)
    {
        const math::Matrix& world_transform = m_transform_system->GetWorld(m_entity_id);
        const math::Vector& world_position = math::GetPosition(world_transform);

        const float angle = math::AngleBetweenPoints(g_player_one.position, world_position) + math::PI_2();
        game::WeaponState fire_state = m_weapon->Fire(world_position, angle, update_context.timestamp);
        if(fire_state == game::WeaponState::FIRE)
            m_n_attacks++;
    
        m_attack_timer = 0;
    }
}
